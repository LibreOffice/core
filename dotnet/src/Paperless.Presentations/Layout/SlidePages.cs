using Paperless.Core.Documents;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;

namespace Paperless.Presentations.Layout;

/// <summary>
/// The pages a deck lays out to: one per slide, in presentation order.
/// </summary>
/// <remarks>
/// <para>
/// The presentation family's answer to "what is a page", and the simplest of the three: a deck
/// states its slide size once and every slide is that size, so pagination has nothing to decide.
/// What a word processor's paginator spends its time on — where the content breaks — has no
/// equivalent here, which is why this is an adapter and not an engine.
/// </para>
/// <para>
/// Hidden slides are kept. They are pages of the document, and dropping them here would make the
/// index a caller sees disagree with the index the file states; whether to render one is a
/// question about output rather than about layout, and <see cref="LaidOutSlide.IsHidden"/> is
/// what answers it. LibreOffice's own PDF export leaves them out, so a comparison against it has
/// to skip them explicitly — which is visible, where a silently shorter sequence would not be.
/// </para>
/// </remarks>
public sealed class SlidePages : IPageSequence
{
    private readonly List<SlideView> _pages;

    /// <summary>Creates a sequence over slides that have been laid out.</summary>
    /// <param name="slides">The slides, in presentation order.</param>
    public SlidePages(IReadOnlyList<LaidOutSlide> slides)
    {
        ArgumentNullException.ThrowIfNull(slides);
        _pages = [.. slides.Select(slide => new SlideView(slide))];
    }

    /// <inheritdoc/>
    public int Count => _pages.Count;

    /// <inheritdoc/>
    public IPage this[int index] => _pages[index];

    /// <summary>The laid-out slides, with the shapes a renderer or a test needs.</summary>
    public IReadOnlyList<LaidOutSlide> Slides => [.. _pages.Select(page => page.Slide)];

    private sealed class SlideView(LaidOutSlide slide) : IPage
    {
        internal LaidOutSlide Slide { get; } = slide;

        /// <inheritdoc/>
        public int Index => Slide.Index;

        /// <inheritdoc/>
        public DocSize Size => Slide.Size;

        /// <inheritdoc/>
        /// <remarks>
        /// The slide's own name when it has one, which PresentationML and ODF both make optional
        /// — LibreOffice's PPTX export writes none at all. Manufacturing "Slide 3" would claim
        /// the file said something it did not, so an unnamed slide has a null label and the
        /// index remains the way to refer to it.
        /// </remarks>
        public string? Label => Slide.Name;

        /// <inheritdoc/>
        public void Draw(IDrawingSink sink) => SlideDrawing.Draw(Slide, sink);
    }
}
