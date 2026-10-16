namespace XRay.Office.WordProcessing.Ooxml;

/// <summary>
/// What a document's <c>FILENAME</c> and <c>TITLE</c> fields evaluate to.
/// </summary>
/// <remarks>
/// <para>
/// Two values rather than the whole of <c>DocumentMetadata</c>, because these are the two a layout can
/// substitute for a cached field result and the two LibreOffice re-evaluates on load. Passing the
/// metadata wholesale would invite a reader to compute the rest, and every other field's cached result
/// is a better answer than anything we could recompute — a <c>DOCPROPERTY</c> can name a property that
/// no longer exists, and a <c>SAVEDATE</c> is about a save that did not happen here.
/// </para>
/// <para>
/// A <see langword="default"/> value substitutes nothing, which is what a caller laying out a document
/// read from a nameless stream gets. That is the lenient reading: the cached result is stale but it is
/// what a reader saw, and drawing nothing would be worse.
/// </para>
/// <para>
/// <strong>A known divergence, recorded rather than hidden.</strong> LibreOffice draws an empty
/// <c>TITLE</c> for a package that states no <c>dc:title</c>, and this leaves the cached result there
/// instead. <c>DocumentMetadata.Title</c> is null for an absent element and for an empty one alike, so
/// the two cases cannot be told apart from it, and keeping what the producer wrote is the lenient half
/// of the choice. It costs parity only on a document that has a <c>TITLE</c> field, a cached result for
/// it, and no title — none in this corpus.
/// </para>
/// </remarks>
/// <param name="FileName">
/// The leaf name of the file the document was read from, extension included, or null when it was read
/// from a stream. LibreOffice's default <c>FILENAME</c> format is <c>NAME_AND_EXT</c>
/// (<c>DomainMapper_Impl.cxx</c>:8300), which is the leaf name and not the path.
/// </param>
/// <param name="Title">The document's title, or null when it states none.</param>
public readonly record struct ConstantFields(string? FileName = null, string? Title = null);
