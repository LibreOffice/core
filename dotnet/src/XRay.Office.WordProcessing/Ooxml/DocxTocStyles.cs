using System.Xml.Linq;

namespace XRay.Office.WordProcessing.Ooxml;

/// <summary>
/// The built-in heading styles a <c>TOC</c> field's <c>\t</c> switch names, whose paragraphs then
/// draw <em>none</em> of their own direct paragraph formatting.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Off by default: this is a LibreOffice behaviour, not a rule of the format, and this
/// project wants Word parity.</strong> Word honours the direct formatting; 26.2.4.2 throws it
/// away. The rule below is measured and implemented so the difference is understood and can be
/// switched back on for a run scored against LibreOffice, but nothing reaches it unless
/// <see cref="WordParity.Variable"/> says so. What that costs on the corpus is written down in
/// <c>dotnet/TODO.word-parity.md</c> — one document, one page.
/// </para>
/// <para>
/// A <c>TOC</c> field carrying
/// <c>\t "Heading 2,1,Heading 3,2"</c> registers those styles as the index's source styles —
/// <c>DomainMapper_Impl::handleToc</c>'s template branch fills <c>LevelParagraphStyles</c> and sets
/// <c>CreateFromLevelParagraphStyles</c>
/// (<c>sw/source/writerfilter/dmapper/DomainMapper_Impl.cxx</c>:7663-7707) — and from then on every
/// paragraph in one of those styles, anywhere in the document, is imported with its <c>w:pPr</c>
/// beyond the <c>w:pStyle</c> discarded. The seat inside Writer that does the discarding is
/// <b>not located</b>; what is below is measured.
/// </para>
/// <para>
/// Measured against 26.2.4.2, each arm a one-attribute variant read out of its own
/// <c>--convert-to fodt</c> so the answer is the importer's and not a rasteriser's
/// (<c>probes/tocstyle-r160/</c>):
/// </para>
/// <list type="bullet">
/// <item><description>
/// <c>\t "Heading 3,2"</c> + a paragraph in the built-in <c>heading 3</c> stating
/// <c>&lt;w:spacing w:after="0"/&gt;</c> — the paragraph comes out with <b>no automatic style at
/// all</b> and keeps the style's 6 pt. The same holds for <c>w:jc</c>, <c>w:ind</c>,
/// <c>w:shd</c>, <c>w:numPr</c> and <c>w:keepNext</c>, and for an <c>after</c> of 480 as much as
/// of 0.
/// </description></item>
/// <item><description>
/// <b>Kept:</b> the paragraph's <c>w:pageBreakBefore</c>, and its runs' own <c>w:rPr</c>.
/// </description></item>
/// <item><description>
/// <b>Controls, all of which keep their direct formatting:</b> a <c>\t</c> naming only
/// <c>Heading 2</c> while the paragraph is <c>heading 3</c>; a <c>\o "1-3"</c> with no <c>\t</c> at
/// all; a paragraph in <c>heading 4</c> while <c>\t</c> names <c>heading 3</c>; a
/// <c>w:customStyle</c> carrying an outline level; a <c>w:customStyle</c> carrying the *name*
/// <c>heading 5</c>; and the built-in <c>Title</c>. So it is the built-in <c>heading N</c> and
/// nothing else.
/// </description></item>
/// <item><description>
/// The paragraph's position does not matter — one written <em>before</em> the <c>TOC</c> field
/// loses its formatting too — and the match is case-insensitive, which the witness needs: it writes
/// <c>\t "… Heading 3,2"</c> where its style sheet says <c>&lt;w:name w:val="heading 3"/&gt;</c>.
/// </description></item>
/// </list>
/// <para>
/// Reach: <b>21 of the 271 corpus DOCX state a <c>\t</c> switch and 7 of them hold a paragraph it
/// voids</b>, 129 paragraphs in all. On <c>24-25_FAA_Holdover_Tables.docx</c> it is the whole of a
/// page: its <c>TABLE 50</c> caption states <c>&lt;w:spacing w:after="0"/&gt;</c> over a style
/// stating 6 pt, and those 6 pt are what push the page-break paragraph under the table onto a page
/// of its own.
/// </para>
/// </remarks>
internal static class DocxTocStyles
{
    /// <summary>How many levels WordprocessingML's built-in headings have.</summary>
    private const int HeadingLevels = 9;

    /// <summary>Whether the reference's behaviour is being reproduced rather than Word's.</summary>
    /// <remarks><see cref="WordParity"/>, which is off by default and is one switch for all of them.</remarks>
    public static bool Enabled => WordParity.ReproduceLibreOffice;

    /// <summary>
    /// The ids of the styles whose paragraphs must ignore their own <c>w:pPr</c>.
    /// </summary>
    /// <param name="body">The <c>w:body</c>, whose fields are scanned for a <c>TOC</c>.</param>
    /// <param name="styles">The style table, for the <c>w:name</c> a switch names a style by.</param>
    /// <returns>
    /// An empty set while <see cref="Enabled"/> is false, which is the default, and an empty set
    /// for a document that states no such switch, which is 250 of the corpus's 271 DOCX.
    /// </returns>
    public static IReadOnlySet<string> Voided(XElement body, WordStyles styles)
        => Enabled ? Resolve(body, styles) : new HashSet<string>(StringComparer.Ordinal);

    /// <summary>
    /// What <see cref="Voided"/> answers when the reference's behaviour is switched on.
    /// </summary>
    /// <remarks>
    /// Separate from the switch so the measurement can be pinned by a test without touching the
    /// environment, which is process-global and would reach every test running beside it.
    /// </remarks>
    internal static IReadOnlySet<string> Resolve(XElement body, WordStyles styles)
    {
        ArgumentNullException.ThrowIfNull(body);
        ArgumentNullException.ThrowIfNull(styles);

        HashSet<string> named = Named(body);
        if (named.Count == 0) return new HashSet<string>(StringComparer.Ordinal);

        HashSet<string> voided = new(StringComparer.Ordinal);
        foreach (WordStyle style in styles.All)
        {
            if (style.Type != WordStyleType.Paragraph || style.IsCustom) continue;
            if (style.Name is not { Length: > 0 } name) continue;
            if (!IsBuiltInHeading(name)) continue;
            if (named.Contains(name) || named.Contains(style.StyleId)) voided.Add(style.StyleId);
        }

        return voided;
    }

    /// <summary>
    /// The paragraph properties such a paragraph keeps, or the element unchanged.
    /// </summary>
    /// <remarks>
    /// A copy rather than an edit, because the tree is the document's and other readers walk it.
    /// Only the four measured survivors are carried over; everything else the <c>w:pPr</c> states is
    /// dropped, which is what the reference does.
    /// </remarks>
    /// <param name="properties">The paragraph's <c>w:pPr</c>, or null.</param>
    /// <param name="voided">The styles from <see cref="Voided"/>.</param>
    public static XElement? Prune(XElement? properties, IReadOnlySet<string> voided)
    {
        if (properties is null || voided.Count == 0) return properties;
        if (Word.Value(properties, "pStyle") is not { Length: > 0 } style) return properties;
        if (!voided.Contains(style)) return properties;

        XElement pruned = new(properties.Name);
        foreach (XElement child in properties.Elements())
        {
            // `w:sectPr` is the section's rather than the paragraph's, and `w:rPr` here is the
            // paragraph mark's character formatting, which the reference keeps.
            if (child.Name.LocalName is "pStyle" or "rPr" or "sectPr" or "pageBreakBefore")
            {
                pruned.Add(child);
            }
        }

        return pruned;
    }

    /// <summary>The style names every <c>TOC … \t "…"</c> in the body lists.</summary>
    /// <remarks>
    /// The switch's argument is <c>name,level</c> pairs separated by commas or semicolons, and only
    /// the names are wanted. An instruction can be split across any number of <c>w:instrText</c>, so
    /// the field markers are tracked rather than each element read on its own — the same walk
    /// <see cref="DocxReferenceFields"/> makes.
    /// </remarks>
    private static HashSet<string> Named(XElement body)
    {
        HashSet<string> named = new(StringComparer.OrdinalIgnoreCase);
        System.Text.StringBuilder? open = null;
        int depth = 0;

        foreach (XElement element in body.Descendants())
        {
            switch (element.Name.LocalName)
            {
                case "instrText":
                    open?.Append(element.Value);
                    break;

                case "fldChar":
                    switch (Word.Attribute(element, "fldCharType"))
                    {
                        case "begin":
                            // Only the outermost field's instruction is accumulated: a `TOC` never
                            // nests inside another field, and its *result* is full of `PAGEREF`.
                            if (depth++ == 0) open = new System.Text.StringBuilder();
                            break;

                        case "separate":
                        case "end":
                            if (open is not null) Take(open.ToString());
                            if (Word.Attribute(element, "fldCharType") == "end" && depth > 0)
                            {
                                if (--depth == 0) open = null;
                            }

                            break;
                    }

                    break;

                case "fldSimple":
                    Take(Word.Attribute(element, "instr") ?? "");
                    break;
            }
        }

        return named;

        void Take(string instruction)
        {
            if (!string.Equals(FieldInstructions.Name(instruction), "TOC",
                               StringComparison.OrdinalIgnoreCase))
            {
                return;
            }

            foreach (string template in Templates(instruction))
            {
                string[] parts = template.Split(',', ';');

                // name, level, name, level … — a trailing name with no level is not a pair and the
                // reference does not read one, so the step is two from the start.
                for (int i = 0; i + 1 < parts.Length; i += 2)
                {
                    string name = parts[i].Trim();
                    if (name.Length > 0) named.Add(name);
                }
            }
        }
    }

    /// <summary>Every <c>\t "…"</c> argument in one instruction.</summary>
    private static IEnumerable<string> Templates(string instruction)
    {
        for (int at = 0; (at = instruction.IndexOf("\\t", at, StringComparison.Ordinal)) >= 0; at += 2)
        {
            // `\t` and not `\tag`: a switch is one letter.
            if (at + 2 < instruction.Length && char.IsAsciiLetter(instruction[at + 2])) continue;

            int quote = instruction.IndexOf('"', at + 2);
            if (quote < 0) yield break;

            int close = instruction.IndexOf('"', quote + 1);
            if (close < 0) yield break;

            yield return instruction[(quote + 1)..close];
            at = close;
        }
    }

    /// <summary>Whether a <c>w:name</c> is one of the nine built-in heading names.</summary>
    /// <remarks>
    /// Word writes <c>heading 3</c> and the switch says <c>Heading 3</c>, so the comparison folds
    /// case. Nothing else is accepted: the measurement says a custom style carrying this very name
    /// is <em>not</em> affected, and <see cref="WordStyle.IsCustom"/> is what separates them.
    /// </remarks>
    private static bool IsBuiltInHeading(string name)
    {
        for (int level = 1; level <= HeadingLevels; level++)
        {
            if (string.Equals(name, "heading " + level, StringComparison.OrdinalIgnoreCase))
            {
                return true;
            }
        }

        return false;
    }
}
