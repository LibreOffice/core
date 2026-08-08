using System.Xml.Linq;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.WordProcessing.Layout;

namespace Paperless.WordProcessing.Ooxml;

/// <content>
/// Reading <c>w:pBdr</c> — the rules round a paragraph, which draw and also take room.
/// </content>
/// <remarks>
/// <para>
/// Two properties of it decide the whole design, and both were measured off LibreOffice's own PDFs rather
/// than taken from the specification.
/// </para>
/// <list type="bullet">
///   <item>
///     <b>It is inherited side by side.</b> A paragraph whose style states a bottom rule and which states
///     only a top of its own keeps both — <c>PBorderHandler</c> turns each side into its own property, so
///     the style chain merges them individually exactly as <c>w:spacing</c>'s attributes do. Taking the
///     innermost <c>w:pBdr</c> whole instead loses every side the paragraph did not restate, which on this
///     corpus is most of them: 294 of the 1078 <c>w:pBdr</c> elements in the words track are in
///     <c>styles.xml</c>.
///   </item>
///   <item>
///     <b>A stated <c>w:val="none"</c> is an answer, not a silence.</b> It beats the style chain below it
///     and takes no room, which is how a header paragraph switches off the rule its style would give it —
///     measured on a probe where the style's bottom rule vanishes and the paragraph below moves up by
///     exactly the allowance.
///   </item>
/// </list>
/// <para>
/// <c>w:space</c> is in <em>points</em> here, which is the one place in WordprocessingML that unit appears
/// on a border: <c>w:sz</c> beside it is in eighths of a point and every length in <c>w:tblBorders</c> is
/// twips. Reading it as twips puts a 10 pt distance at half a point.
/// </para>
/// </remarks>
public sealed partial class DocxLayoutSource
{
    /// <summary>
    /// The rules round a paragraph, resolved side by side down its style chain, or null when it has none.
    /// </summary>
    /// <param name="properties">The paragraph's own <c>w:pPr</c>, or null.</param>
    private ParagraphBorderSet? ParagraphBorders(XElement? properties)
    {
        string? styleId = Word.Attribute(Word.Child(properties, "pStyle"), "val")
                          ?? _styles.DefaultStyleId(WordStyleType.Paragraph);

        List<XElement> layers = _styles.ParagraphPropertyLayers(
            "pBdr", properties, styleId, _tableStyle);

        if (layers.Count == 0) return null;

        ParagraphBorderSet set = new()
        {
            Left = Side("start", "left"),
            Right = Side("end", "right"),
            Top = Side("top", null),
            Bottom = Side("bottom", null),
            Between = Side("between", null),
        };

        return set.IsEmpty ? null : set;

        ParagraphBorder? Side(string side, string? legacySide)
        {
            foreach (XElement layer in layers)
            {
                if (ParagraphBorderOf(layer, side, legacySide) is { } found) return found;
            }

            return null;
        }
    }

    /// <summary>
    /// One side of a <c>w:pBdr</c>, or null when that layer states nothing about it.
    /// </summary>
    /// <remarks>
    /// A zero width and a null are different answers: the first is a stated <c>none</c> that stops the
    /// search, and the second lets the layer below speak. <c>w:sz</c> is in eighths of a point and
    /// <c>w:space</c> in whole points; a border whose colour resolves to nothing is still a border, so
    /// black is the fallback.
    /// </remarks>
    private ParagraphBorder? ParagraphBorderOf(XElement? borders, string side, string? legacySide)
    {
        XElement? stated =
            Word.Child(borders, side)
            ?? (legacySide is null ? null : Word.Child(borders, legacySide));

        if (stated is null) return null;

        Length space =
            Word.Integer(Word.Attribute(stated, "space"), out int points) && points > 0
                ? Length.FromPoints(points)
                : Length.Zero;

        if (Word.Attribute(stated, "val") is null or "none" or "nil")
        {
            return new ParagraphBorder(Length.Zero, Length.Zero, Colour.Black);
        }

        Length width =
            Word.Integer(Word.Attribute(stated, "sz"), out int eighths) && eighths > 0
                ? Length.FromPoints(eighths / 8.0)
                : HairlineBorder;

        Colour colour =
            WordThemeColour.Read(stated, _theme, "color", "themeColor", "themeTint", "themeShade")
            ?? Colour.Black;

        return new ParagraphBorder(width, space, colour);
    }

    /// <summary>
    /// Merges each run of identically bordered consecutive paragraphs into one box.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Word and Writer both draw two adjacent paragraphs carrying the same <c>w:pBdr</c> as a single box:
    /// no rule between them, and neither the lower one's top allowance nor the upper one's bottom
    /// allowance is spent. Measured — two such paragraphs sit one plain line pitch apart, 12.65 pt, where
    /// two separately boxed ones would be 6.50 pt further.
    /// </para>
    /// <para>
    /// Done here rather than in the paginator because the join changes the picture <em>and</em> the height,
    /// and the two have to agree: a border drawn where no room was reserved lands on the text below it.
    /// </para>
    /// <para>
    /// A table between two bordered paragraphs breaks the run, which is what the type test does — only a
    /// <see cref="PageParagraph"/> can join, and anything else resets.
    /// </para>
    /// </remarks>
    /// <param name="blocks">The blocks of one flow, in order. Rewritten in place.</param>
    internal static void JoinParagraphBorders(List<PageBlock> blocks)
    {
        ArgumentNullException.ThrowIfNull(blocks);

        // The join is decided from what the reader stated, never from what an earlier step of this walk
        // has already rewritten: the second paragraph of a run of three has had its top removed by the
        // time the third is examined, and comparing that against the third's untouched set would find
        // them different and break the run in the middle.
        ParagraphBorderSet?[] stated =
            [.. blocks.Select(block => (block as PageParagraph)?.Borders)];

        for (int i = 0; i + 1 < blocks.Count; i++)
        {
            if (stated[i] is not { } borders || !borders.JoinsWith(stated[i + 1])) continue;
            if (blocks[i] is not PageParagraph upper || blocks[i + 1] is not PageParagraph lower) continue;

            (ParagraphBorderSet above, ParagraphBorderSet below) = ParagraphBorderSet.Join(borders);

            // Only the sides the join settles are taken from the pair, so a paragraph in the middle of a
            // run keeps the top the previous step already removed.
            blocks[i] = upper with
            {
                Borders = (upper.Borders ?? borders) with { Bottom = above.Bottom, Between = null },
            };

            blocks[i + 1] = lower with
            {
                Borders = (lower.Borders ?? borders) with
                {
                    Top = below.Top, Between = null, JoinsAbove = true,
                },
            };
        }
    }
}
