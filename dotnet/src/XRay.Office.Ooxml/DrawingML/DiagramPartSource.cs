using System.Xml.Linq;

namespace XRay.Office.Ooxml.DrawingML;

/// <summary>
/// The two lookups a SmartArt diagram needs from the package that holds it: turning a
/// relationship id stated on one part into the name of another, and loading a part by name.
/// </summary>
/// <remarks>
/// <para>
/// A diagram is the same five parts in a deck, a document and a workbook, and nothing about
/// resolving them is family-specific — so this stands where the resolution does, rather than
/// each family re-deriving it against its own package type. It is two delegates and not an
/// interface because that is the whole of the dependency: every family already has a package
/// object that answers both, and neither has any business knowing what a diagram is.
/// </para>
/// <para>
/// <strong>The relationship is scoped to the part that states it</strong>, which is why
/// <see cref="Target"/> takes the owning part's name rather than resolving against the package's
/// main part. A <c>dgm:relIds</c> in <c>word/document.xml</c> resolves against
/// <c>word/_rels/document.xml.rels</c>, one in a header against that header's own, and one on a
/// slide against the slide's. Resolving against the wrong part does not fail — it silently finds
/// whatever that part happens to call <c>rId9</c>.
/// </para>
/// </remarks>
public sealed class DiagramPartSource
{
    private readonly Func<string, string, string?> _target;
    private readonly Func<string, XElement?> _load;

    /// <summary>Creates a source over a package's own resolution and loading.</summary>
    /// <param name="target">
    /// Given the part stating a relationship and the relationship's id, the name of the part it
    /// names — or null when there is no such relationship or it points outside the package.
    /// </param>
    /// <param name="load">
    /// Given a part name, that part's root element — or null when the package has no such part or
    /// it does not parse. Expected to cache, since a diagram asks for its data model twice.
    /// </param>
    public DiagramPartSource(Func<string, string, string?> target, Func<string, XElement?> load)
    {
        ArgumentNullException.ThrowIfNull(target);
        ArgumentNullException.ThrowIfNull(load);
        _target = target;
        _load = load;
    }

    /// <summary>The part a relationship names, or null when it names none inside the package.</summary>
    /// <param name="partName">The part whose relationships the id is scoped to.</param>
    /// <param name="relationshipId">The <c>r:id</c> value, which may be null or absent.</param>
    /// <returns>The target part's name, or null.</returns>
    public string? Target(string partName, string? relationshipId)
        => partName is null || string.IsNullOrEmpty(relationshipId)
            ? null
            : _target(partName, relationshipId);

    /// <summary>A part's root element, or null when it is missing or unreadable.</summary>
    /// <param name="partName">The part's name inside the package.</param>
    /// <returns>The root element, or null.</returns>
    public XElement? Load(string partName)
        => string.IsNullOrEmpty(partName) ? null : _load(partName);
}
