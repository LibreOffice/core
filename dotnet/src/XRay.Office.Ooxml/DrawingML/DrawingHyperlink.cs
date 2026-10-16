using System.Xml.Linq;

namespace XRay.Office.Ooxml.DrawingML;

/// <summary>
/// What an <c>a:hlinkClick</c> on a run's <c>a:rPr</c> makes of that run.
/// </summary>
/// <remarks>
/// <para>
/// A DrawingML hyperlink is not a character property. <c>TextRun::insertAt</c> takes one of two
/// branches on whether the run's own hyperlink property map is empty
/// (<c>oox/source/drawingml/textrun.cxx</c>:88): with it empty the run's text is inserted as
/// text, and with anything in it the run becomes a
/// <c>com.sun.star.text.TextField.URL</c> whose <c>Representation</c> is the run's own
/// <c>a:t</c> (<c>:149-157</c>) — one field per <c>a:r</c>, whatever the link is split into.
/// The hyperlink colour and the automatic underline are set in the same branch (<c>:162-168</c>),
/// so they follow the field rather than the element.
/// </para>
/// <para>
/// <strong>An <c>a:hlinkClick</c> is therefore not always a hyperlink.</strong> The map is filled
/// by <c>HyperLinkContext</c> (<c>oox/source/drawingml/hyperlinkcontext.cxx</c>:40-156), which
/// sets a property only for what the element actually states — a resolvable <c>r:id</c>, a
/// <c>tooltip</c>, a <c>tgtFrame</c>, an <c>action</c>, an <c>invalidUrl</c>, <c>history</c> off,
/// <c>highlightClick</c> or <c>endSnd</c> on, or an <c>a:extLst</c> child, which sets the
/// character colour (<c>:167-169</c>). An element that states none of them, which is what
/// PowerPoint writes to <em>clear</em> an inherited link — <c>&lt;a:hlinkClick r:id=""/&gt;</c> —
/// leaves the map empty and the run is ordinary text: no field, no link colour, no underline.
/// Measured on <c>probes/pptx-field-r82/</c>'s <c>v10</c>, which 26.2.4.2 draws in black at the
/// word-broken positions, against <c>v11</c> — the same element plus a <c>tooltip</c> — which it
/// draws blue and cell-broken.
/// </para>
/// <para>
/// <strong>A stated <c>r:id</c> is taken to resolve.</strong> Deciding otherwise needs the part's
/// relationships, which the layout reader does not carry; the approximation is exact on the
/// corpus, where all 608 text-run <c>a:hlinkClick</c> of the 251 decks and 24 workbooks that
/// state a non-empty <c>r:id</c> resolve to a target, and the only element in the corpus that
/// leaves the map empty states <c>r:id=""</c> and nothing else.
/// </para>
/// </remarks>
public static class DrawingHyperlink
{
    /// <summary>
    /// Whether an <c>a:hlinkClick</c> leaves at least one property on the run's hyperlink
    /// property map, and so turns the run into a field.
    /// </summary>
    /// <param name="hyperlink">The <c>a:hlinkClick</c> element, or null when the run has none.</param>
    public static bool MakesField(XElement? hyperlink)
    {
        if (hyperlink is null) return false;

        if (!string.IsNullOrEmpty(Drawing.RelationshipId(hyperlink))) return true;

        foreach (string stated in Stated)
        {
            if (!string.IsNullOrEmpty(Drawing.Attribute(hyperlink, stated))) return true;
        }

        // `history` defaults to true and is recorded only when it is false; the other two default
        // to false and are recorded only when they are true (hyperlinkcontext.cxx:145-155).
        if (Drawing.Flag(hyperlink, "history") == false) return true;
        if (Drawing.Flag(hyperlink, "highlightClick") == true) return true;
        if (Drawing.Flag(hyperlink, "endSnd") == true) return true;

        return Drawing.Child(hyperlink, "extLst") is not null;
    }

    /// <summary>
    /// Whether the link supplies the run's colour itself, so that the theme's <c>hlink</c> slot
    /// does not.
    /// </summary>
    /// <remarks>
    /// <c>textrun.cxx</c>:162 sets the scheme colour only when the map holds no <c>CharColor</c>,
    /// and the one thing that puts one there is an <c>a:extLst</c> child
    /// (<c>hyperlinkcontext.cxx</c>:167-169).
    /// </remarks>
    /// <param name="hyperlink">The <c>a:hlinkClick</c> element, or null when the run has none.</param>
    public static bool StatesItsOwnColour(XElement? hyperlink)
        => Drawing.Child(hyperlink, "extLst") is not null;

    /// <summary>The attributes whose mere presence fills a slot on the map.</summary>
    private static readonly string[] Stated =
        ["tooltip", "tgtFrame", "action", "invalidUrl"];
}
