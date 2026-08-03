using System.Xml.Linq;
using Paperless.Core.Units;
using Paperless.OpenDocument;
using Paperless.OpenDocument.Styles;
using Paperless.Text.Fonts;
using Paperless.Text.Layout;
using Paperless.Text.Shaping;
using Paperless.WordProcessing.Layout;

namespace Paperless.WordProcessing.OpenDocument;

/// <summary>
/// The list half of the ODF layout walk: which level a paragraph is at, what its label says, and
/// where the level puts both.
/// </summary>
/// <remarks>
/// <para>
/// ODF states list <em>structure</em> by nesting <c>text:list</c> elements and list <em>appearance</em>
/// in a separately named list style, so the level is the nesting depth and nothing on the paragraph says
/// it. That is the opposite of the flat "paragraph plus <c>w:ilvl</c>" the Word formats use, and it is
/// why the counters have to be carried by the walk rather than resolved per paragraph.
/// </para>
/// <para>
/// The same counting <see cref="Paperless.OpenDocument.OdfContentReader"/> does for extraction, repeated
/// here rather than shared, because the two walks are separate passes over the same file and neither may
/// advance the other's counters — extraction reads headers and footers as well, and a page number in a
/// footer must not continue the body's list.
/// </para>
/// </remarks>
public sealed partial class OdtLayoutSource
{
    /// <summary>The deepest list nesting ODF defines.</summary>
    private const int MaxListLevel = 10;

    /// <summary>The counter at every level, indexed by level minus one.</summary>
    private readonly List<int> _listCounters = [];

    /// <summary>The nesting depth of the walk, counted from one; zero means no list.</summary>
    private int _listLevel;

    /// <summary>The list style in force, from the innermost enclosing list that named one.</summary>
    private OdfListStyle? _listStyle;

    /// <summary>
    /// Whether the next paragraph is the one that carries its item's label.
    /// </summary>
    /// <remarks>
    /// Only the first <c>text:p</c> of a <c>text:list-item</c> does. The rest are continuation
    /// paragraphs, which keep the level's indents and draw nothing — <c>text-features.odt</c>'s
    /// "Continuation paragraph of item two" is one, and LibreOffice sets it at the item's text indent
    /// with no number in front.
    /// </remarks>
    private bool _labelPending;

    /// <summary>
    /// Enters a <c>text:list</c>, returning what has to be put back when the walk leaves it.
    /// </summary>
    /// <remarks>
    /// A list restarts its level's count unless it says otherwise. <c>text:continue-numbering</c> asks
    /// for the previous list at this level to be continued and <c>text:continue-list</c> names which one;
    /// either is enough to keep the counter.
    /// </remarks>
    private (int Level, OdfListStyle? Style) EnterList(XElement list)
    {
        (int Level, OdfListStyle? Style) outer = (_listLevel, _listStyle);

        int level = Math.Min(_listLevel + 1, MaxListLevel);
        OdfListStyle? style =
            _styles.FindListStyle(list.Attribute(XName.Get("style-name", OdfNamespaces.Text))?.Value)
            ?? _listStyle;

        bool continues =
            OdfValue.ParseBoolean(
                list.Attribute(XName.Get("continue-numbering", OdfNamespaces.Text))?.Value) == true
            || list.Attribute(XName.Get("continue-list", OdfNamespaces.Text)) is not null;

        _listLevel = level;
        _listStyle = style;
        if (!continues) ResetCounter(level);

        return outer;
    }

    /// <summary>Puts back the state <see cref="EnterList"/> replaced.</summary>
    private void LeaveList((int Level, OdfListStyle? Style) outer)
        => (_listLevel, _listStyle) = outer;

    /// <summary>
    /// Starts a <c>text:list-item</c> or a <c>text:list-header</c>.
    /// </summary>
    /// <param name="item">The element, for its <c>text:start-value</c>.</param>
    /// <param name="numbered">
    /// False for a <c>text:list-header</c>, which is unnumbered introductory text at this level: it
    /// neither advances the counter nor draws a label.
    /// </param>
    private void BeginListItem(XElement item, bool numbered)
    {
        if (_listLevel < 1) return;

        if (numbered)
        {
            if (OdfValue.ParseInt(item.Attribute(XName.Get("start-value", OdfNamespaces.Text))?.Value)
                is { } start)
            {
                SetCounter(_listLevel, start - 1);
            }
            IncrementCounter(_listLevel);
        }

        _labelPending = numbered;
    }

    private void ResetCounter(int level)
    {
        EnsureCounters(level);
        _listCounters[level - 1] = (_listStyle?.GetLevel(level)?.StartValue ?? 1) - 1;
    }

    private void SetCounter(int level, int value)
    {
        EnsureCounters(level);
        _listCounters[level - 1] = value;
    }

    private void IncrementCounter(int level)
    {
        EnsureCounters(level);
        _listCounters[level - 1]++;

        // Entering an item resets everything below it, so a second top-level item's sub-list starts from
        // one again rather than continuing the first item's.
        for (int deeper = level; deeper < _listCounters.Count; deeper++)
        {
            _listCounters[deeper] = (_listStyle?.GetLevel(deeper + 1)?.StartValue ?? 1) - 1;
        }
    }

    private void EnsureCounters(int level)
    {
        while (_listCounters.Count < level) _listCounters.Add(0);
    }

    /// <summary>
    /// The label and the indents the level in force gives a paragraph.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The indents apply to every paragraph of an item and the label only to its first, which is why the
    /// two come back together: a continuation paragraph still has to line up under the text above it.
    /// </para>
    /// <para>
    /// <strong>ODF says the geometry two ways and the attribute names do not admit it.</strong> The older
    /// <em>label-width-and-position</em> mode states <c>text:space-before</c> and
    /// <c>text:min-label-width</c>, whose sum is the text's indent and whose second member is the hanging
    /// distance — the mapping <c>xmloff/source/style/xmlnumi.cxx:433</c> makes. The newer
    /// <em>label-alignment</em> mode states <c>fo:margin-left</c> and <c>fo:text-indent</c> directly, in a
    /// <c>style:list-level-label-alignment</c> child, and adds a tab stop for the label to be followed by.
    /// Reading only the second — which is what the attribute names invite, since they are the ones a
    /// paragraph style uses — leaves every list in a LibreOffice-written document flush with the margin.
    /// </para>
    /// </remarks>
    /// <param name="format">The paragraph's own resolved formatting.</param>
    /// <param name="text">Its resolved text style, for the size a relative label size is a fraction of.</param>
    /// <param name="face">Its face, which the label falls back to when the level names none.</param>
    /// <param name="wantsLabel">Whether this paragraph is the one that carries the item's label.</param>
    private (PageLabel? Label, ParagraphFormat Format) ListFormatting(
        ParagraphFormat format, OdfTextStyle text, OpenTypeFace face, bool wantsLabel)
    {
        if (_listLevel < 1 || _listStyle is null) return (null, format);
        if (_listStyle.GetLevel(_listLevel) is not { } definition) return (null, format);

        OdfPropertySet? properties = definition.LevelProperties;
        XElement? alignment = properties?.Child(OdfNamespaces.Style, "list-level-label-alignment");

        Length tabStop = Length.Zero;
        LabelFollow follow;
        Length minimumGap = Length.Zero;
        Length ownFirstLine = format.FirstLineIndent;

        if (alignment is not null)
        {
            // Label-alignment mode replaces the paragraph's indents rather than adding to them, which is
            // what `SwTextNode::GetLeftMarginWithNum` does when the node has no hard-set indent of its
            // own (`sw/source/core/txtnode/ndtxt.cxx:3439`).
            format = format with
            {
                StartIndent =
                    Measure(alignment, OdfNamespaces.FoCompatible, "margin-left") ?? format.StartIndent,
                FirstLineIndent =
                    Measure(alignment, OdfNamespaces.FoCompatible, "text-indent")
                    ?? format.FirstLineIndent,
            };

            follow = alignment.Attribute(XName.Get("label-followed-by", OdfNamespaces.Text))?.Value switch
            {
                "space" => LabelFollow.Space,
                "nothing" => LabelFollow.Nothing,
                _ => LabelFollow.ListTab,
            };
            tabStop = Measure(alignment, OdfNamespaces.Text, "list-tab-stop-position") ?? Length.Zero;
        }
        else
        {
            Length space = Property(properties, "space-before");
            Length labelWidth = Property(properties, "min-label-width");
            minimumGap = Property(properties, "min-label-distance");

            format = format with
            {
                StartIndent = format.StartIndent + space + labelWidth,
                FirstLineIndent = format.FirstLineIndent - labelWidth,
            };

            // No stop to tab to: the room the level set aside is the whole of the answer, and Writer
            // fills it by widening the number portion (`porfld.cxx:653`).
            follow = LabelFollow.Nothing;
        }

        // A paragraph that draws no label takes the level's left margin and none of its hanging indent:
        // `SwTextNode::GetFirstLineOfsWithNum` (`ndtxt.cxx:3469`) applies the numbering's offset only when
        // the node `IsCountedInList`, so a continuation paragraph lines up under the text above it rather
        // than hanging into the space where the number would have gone. Measured on `text-features.odt`:
        // "Continuation paragraph of item two" starts at 74.8 pt, not 56.7 pt.
        if (!wantsLabel) return (null, format with { FirstLineIndent = ownFirstLine });
        if (_listStyle.FormatLabel(_listLevel, _listCounters) is not { Length: > 0 } drawn)
            return (null, format);

        Length size = LabelSize(definition, text);

        // A bullet level names a symbol font and a numbered one usually names nothing, in which case the
        // label is set in the item's own face — which is what makes "1." match the text beside it.
        OpenTypeFace labelFace = definition.Typeface is { Length: > 0 } family
            ? Face(family, text.Weight, text.IsItalic) ?? face
            : face;

        PageLabel label = PageLabel.Measured(
            drawn, labelFace, size,
            new ShapingOptions(Language: text.Language, DisableKerning: !text.AutoKerning));

        return (
            label with
            {
                Font = _references.GetValueOrDefault((labelFace.FamilyName, text.Weight, text.IsItalic)),
                Colour = text.Colour ?? Core.Graphics.Colour.Black,
                Follow = follow,
                TabStop = tabStop,
                MinimumGap = minimumGap,
            },
            format);

        static Length Property(OdfPropertySet? set, string name)
            => OdfValue.ParseLength(set?.Get(OdfNamespaces.Text, name)) ?? Length.Zero;

        static Length? Measure(XElement element, string ns, string name)
            => OdfValue.ParseLength(element.Attribute(XName.Get(name, ns))?.Value);
    }

    /// <summary>
    /// The size the label is set at: the level's own when it states one, and the item's text otherwise.
    /// </summary>
    /// <remarks>
    /// <para>
    /// ODF says it two ways and both are real. The one already handled is a <em>percentage</em> written
    /// straight on the level — <c>fo:font-size="45%"</c>, which is what every list style LibreOffice
    /// generates carries. The other is <c>text:style-name</c>, a named character style holding the
    /// label's whole character formatting, and it is what LibreOffice's own WW8 import produces: a
    /// <c>.doc</c> whose list level sets 12 pt over 11 pt text round-trips to a <c>WW8Num1z0</c>
    /// character style with <c>fo:font-size="12pt"</c> and a level naming it.
    /// </para>
    /// <para>
    /// <strong>Only the size is taken from it.</strong> The style also names a family, and for a bullet
    /// level that family is a symbol font whose code point <see cref="OdfListStyle"/> has already
    /// normalised — the same reason the WW8 reader takes a level's size and not its face. A level that
    /// really means its face states it in <c>style:font-name</c> on the level itself, which
    /// <see cref="OdfListLevel.Typeface"/> reads and the caller honours.
    /// </para>
    /// <para>
    /// A label bigger than its item's text raises that item's first line; see
    /// <see cref="Layout.PageParagraph.LabelRaisesFirstLine"/>.
    /// </para>
    /// </remarks>
    private Length LabelSize(OdfListLevel definition, OdfTextStyle text)
    {
        if (definition.RelativeSize is { } fraction) return Length.FromEmu((long)(text.Size.Emu * fraction));

        if (definition.TextStyleName is { Length: > 0 } named
            && OdfParagraphFormats.StatedTextSize(_styles, named) is { } stated
            && stated > Length.Zero)
        {
            return stated;
        }

        return text.Size;
    }
}
