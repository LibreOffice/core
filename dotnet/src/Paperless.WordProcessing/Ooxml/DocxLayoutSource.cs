using System.Globalization;
using System.Text;
using System.Xml.Linq;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.Text.Shaping;
using Paperless.WordProcessing.Layout;

namespace Paperless.WordProcessing.Ooxml;

/// <summary>
/// Turns a DOCX body into the paragraphs the paginator takes.
/// </summary>
/// <remarks>
/// <para>
/// A second walk over <c>document.xml</c>, for the same reason ODF has one: extraction discards the font
/// sizes, indents and spacing layout needs, and making it carry them would charge every caller for a
/// feature most never use.
/// </para>
/// <para>
/// The same gap as the ODF source: per-run font sizes are not honoured, so a paragraph is measured
/// wholly in the font its paragraph mark carries. The tallest run on a line sets that line's height, so
/// a paragraph mixing sizes lays out slightly short until the runs are walked.
/// </para>
/// </remarks>
public sealed class DocxLayoutSource
{
    /// <summary>How many paragraphs are read before the rest are ignored.</summary>
    public const int MaxParagraphs = 200000;

    /// <summary>
    /// The character an anchor occupies: a field result, a note reference, an inline drawing.
    /// </summary>
    /// <remarks>
    /// The same one the document model and the ODF source use, so an offset means the same thing
    /// wherever it was counted.
    /// </remarks>
    private const char AnchorCharacter = '\u0001';

    /// <summary>
    /// The character a <c>w:br</c> becomes.
    /// </summary>
    /// <remarks>
    /// U+2028, whose UAX #14 class is a mandatory break, so the break iterator honours it without layout
    /// special-casing anything. A newline would break the same way but would read as the end of a
    /// paragraph to anything that later scans the text, which a break inside one is not.
    /// </remarks>
    private const char LineSeparator = '\u2028';

    private readonly WordStyles _styles;
    private readonly SystemFontResolver _fonts;
    private readonly Length _defaultTabInterval;
    private readonly Dictionary<(string? Family, int Weight, bool Italic), OpenTypeFace?> _faces = [];
    private readonly Dictionary<(string? Family, int Weight, bool Italic), FontReference> _references =
        [];

    /// <summary>Creates a source over a document's styles and settings.</summary>
    /// <param name="styles">The document's styles, including its <c>w:docDefaults</c>.</param>
    /// <param name="settings">The document's <c>w:settings</c> root, or null.</param>
    /// <param name="fonts">The font resolver, or null to build one over the installed fonts.</param>
    public DocxLayoutSource(
        WordStyles styles, XElement? settings = null, SystemFontResolver? fonts = null)
    {
        ArgumentNullException.ThrowIfNull(styles);
        _styles = styles;
        _fonts = fonts ?? new SystemFontResolver(SystemFontIndex.Build());
        _defaultTabInterval = TabInterval(settings);
    }

    /// <summary>The substitutions made while resolving the document's fonts.</summary>
    public IReadOnlyList<FontSubstitution> Substitutions => _fonts.Substitutions;

    /// <summary>Reads the body's paragraphs, in document order.</summary>
    /// <param name="body">The <c>w:body</c> element.</param>
    public List<PageParagraph> Read(XElement body)
    {
        ArgumentNullException.ThrowIfNull(body);

        List<PageParagraph> paragraphs = [];
        Walk(body, paragraphs, depth: 0);
        return paragraphs;
    }

    /// <summary>
    /// Walks the body's block-level children.
    /// </summary>
    /// <remarks>
    /// <c>w:sdt</c> — a structured-document tag, which is what a content control is — wraps ordinary
    /// content inside a <c>w:sdtContent</c>, so a walk that stopped at it would lose every paragraph in
    /// a form. Tables are skipped rather than flattened, because a table is laid out as a grid and
    /// flattening it would give the page a height no table has.
    /// </remarks>
    private void Walk(XElement element, List<PageParagraph> into, int depth)
    {
        if (depth > 64 || into.Count >= MaxParagraphs) return;

        foreach (XElement child in element.Elements())
        {
            if (into.Count >= MaxParagraphs) return;

            if (Word.Is(child, "p"))
            {
                if (Paragraph(child) is { } paragraph) into.Add(paragraph);
                continue;
            }

            if (Word.Is(child, "sdt") || Word.Is(child, "sdtContent"))
            {
                Walk(child, into, depth + 1);
            }
        }
    }

    private PageParagraph? Paragraph(XElement element)
    {
        XElement? properties = Word.Child(element, "pPr");

        WordTextStyle text = WordParagraphFormats.ResolveText(_styles, properties);
        OpenTypeFace? face = Face(text);
        if (face is null) return null;

        return new PageParagraph
        {
            Text = TextOf(element),
            Face = face,
            Font = _references.GetValueOrDefault(
                (text.FamilyName, text.Weight, text.IsItalic)),
            Format = WordParagraphFormats.Resolve(_styles, properties, _defaultTabInterval),
            EmSize = text.Size,
            Language = text.Language,
            Shaping = new ShapingOptions(Language: text.Language),
            Source = element,
        };
    }

    /// <summary>
    /// A paragraph's text, as laid out rather than as stored.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Two things have to be got right or the measurement is of the wrong string. A <c>w:del</c> holds
    /// text a tracked change removed, and it is still in the file — measuring it lays out words the
    /// document does not show. And a field's instruction lives in the same run sequence as its result,
    /// bracketed by <c>w:fldChar</c> markers, so a reader that takes every <c>w:t</c> lays out
    /// <c>PAGE \* Arabic</c> in the middle of a sentence.
    /// </para>
    /// <para>
    /// <c>w:tab</c> and <c>w:br</c> are elements rather than characters, as in ODF, and dropping them
    /// silently closes up the space they occupy.
    /// </para>
    /// </remarks>
    private static string TextOf(XElement paragraph)
    {
        StringBuilder builder = new();
        bool inInstruction = false;
        Append(paragraph, builder, ref inInstruction, depth: 0);
        return builder.ToString();

        static void Append(XElement element, StringBuilder builder, ref bool inInstruction, int depth)
        {
            if (depth > 64) return;

            foreach (XElement child in element.Elements())
            {
                string name = child.Name.LocalName;

                switch (name)
                {
                    case "del" or "delText" or "instrText":
                        // Deleted text and field instructions are in the file and not on the page.
                        break;

                    case "fldChar":
                        // "separate" ends the instruction and starts the result; "end" closes the field.
                        string? type = Word.Attribute(child, "fldCharType");
                        if (type == "begin") inInstruction = true;
                        else if (type is "separate" or "end") inInstruction = false;
                        break;

                    case "t" when !inInstruction:
                        builder.Append(child.Value);
                        break;

                    case "tab" when !inInstruction:
                        builder.Append('\t');
                        break;

                    case "br" when !inInstruction:
                        builder.Append(LineSeparator);
                        break;

                    case "footnoteReference" or "endnoteReference" or "commentReference"
                        or "drawing" or "pict" or "object":
                        builder.Append(AnchorCharacter);
                        break;

                    case "pPr" or "bookmarkStart" or "bookmarkEnd" or "proofErr" or "rPr":
                        break;

                    default:
                        Append(child, builder, ref inInstruction, depth + 1);
                        break;
                }
            }
        }
    }

    /// <summary>
    /// The document's default tab interval.
    /// </summary>
    /// <remarks>
    /// Half an inch when the document does not say, which is what Word uses. A zero would make a tab
    /// advance nowhere, so it is treated as absent rather than honoured.
    /// </remarks>
    private static Length TabInterval(XElement? settings)
        => Word.Attribute(Word.Child(settings, "defaultTabStop"), "val") is { } text
           && long.TryParse(text, CultureInfo.InvariantCulture, out long twips)
           && twips > 0
            ? Length.FromTwips(twips)
            : Length.FromTwips(720);

    private OpenTypeFace? Face(WordTextStyle text)
    {
        (string? Family, int Weight, bool Italic) key = (text.FamilyName, text.Weight, text.IsItalic);
        if (_faces.TryGetValue(key, out OpenTypeFace? cached)) return cached;

        OpenTypeFace? face = null;
        try
        {
            FontReference reference = _fonts.Resolve(
                new FontRequest(text.FamilyName ?? string.Empty, text.Weight, text.IsItalic));

            face = _fonts.LoadOpenType(reference);
            _references[key] = reference;
        }
        catch (Exception exception) when (exception is Core.MalformedDocumentException
                                             or IOException
                                             or UnauthorizedAccessException)
        {
            // Nothing to measure the paragraph with. Dropping it gives a shorter document rather than
            // an exception out of the middle of a layout.
        }

        _faces[key] = face;
        return face;
    }
}
