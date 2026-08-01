namespace Paperless.Vector.Metafiles;

/// <summary>
/// The handle table a metafile creates, selects and deletes graphics objects through.
/// </summary>
/// <remarks>
/// <para>
/// <b>The semantics matter more than the storage.</b> WMF never states a handle when it creates
/// an object: the handle is the index of the lowest free slot, so a decoder that appends
/// instead assigns different handles from the producer as soon as the file deletes anything —
/// and real files delete aggressively, because GDI limited a device context to a few hundred
/// simultaneous objects. Every subsequent <c>SelectObject</c> then picks the wrong pen. Ported
/// from <c>MtfTools::CreateObject</c> and <c>DeleteObject</c>
/// (<c>emfio/source/reader/mtftools.cxx:1018-1120</c>).
/// </para>
/// <para>
/// Real files also leak: they create objects they never delete, and delete handles they never
/// created. Both are tolerated here, because refusing either would reject files that Word and
/// PowerPoint render without complaint.
/// </para>
/// </remarks>
public sealed class GraphicsObjectTable
{
    /// <summary>
    /// The largest handle the table will hold.
    /// </summary>
    /// <remarks>
    /// A WMF handle is 16 bits, so a file cannot name a larger one; the cap matters because an
    /// EMF handle is 32 bits and a malformed one would otherwise ask for a table of four
    /// billion slots before a single record had drawn anything.
    /// </remarks>
    public const int MaxHandles = 0x10000;

    private readonly List<GraphicsObject?> _slots = [];

    /// <summary>
    /// The first slot index at or above which a free slot is guaranteed to exist.
    /// </summary>
    /// <remarks>
    /// LibreOffice's <c>mnLowestFreeGDIObj</c>. Without it, creating the n-th object is a linear
    /// scan from zero and a file with a few thousand objects becomes quadratic — which is a
    /// denial of service on untrusted input, not merely a slow decode.
    /// </remarks>
    private int _lowestFree;

    /// <summary>How many slots the table has ever needed.</summary>
    public int Count => _slots.Count;

    /// <summary>The object a handle names, or null when the slot is empty or out of range.</summary>
    public GraphicsObject? this[int handle]
        => handle >= 0 && handle < _slots.Count ? _slots[handle] : null;

    /// <summary>
    /// Stores an object in the lowest free slot and answers its handle, as WMF's create records
    /// do.
    /// </summary>
    /// <param name="value">
    /// The object, or <see cref="UnsupportedGraphicsObject.Instance"/> for a record whose kind
    /// is not modelled — never null, because a null would leave the slot free and shift every
    /// later handle.
    /// </param>
    /// <returns>The handle, or -1 when the table is full.</returns>
    public int Add(GraphicsObject value)
    {
        ArgumentNullException.ThrowIfNull(value);

        int index = _lowestFree;
        while (index < _slots.Count && _slots[index] is not null) index++;

        if (index >= MaxHandles) return -1;

        if (index == _slots.Count) _slots.Add(value);
        else _slots[index] = value;

        _lowestFree = index + 1;
        return index;
    }

    /// <summary>
    /// Stores an object at a stated handle, as EMF's create records do.
    /// </summary>
    /// <remarks>
    /// Growing the table to reach a high handle is deliberate and bounded by
    /// <see cref="MaxHandles"/>: EMF files routinely create object 40 without ever having
    /// created 0 to 39.
    /// </remarks>
    /// <param name="handle">The handle to store at.</param>
    /// <param name="value">The object, or null to leave the slot free.</param>
    public void Set(int handle, GraphicsObject? value)
    {
        if (handle < 0 || handle >= MaxHandles) return;

        while (_slots.Count <= handle) _slots.Add(null);

        _slots[handle] = value;

        if (value is null)
        {
            if (handle < _lowestFree) _lowestFree = handle;
        }
        else if (handle == _lowestFree)
        {
            _lowestFree = handle + 1;
        }
    }

    /// <summary>
    /// Frees a handle so that the next created object may reuse it.
    /// </summary>
    /// <remarks>
    /// Deleting the object a device context currently has selected is legal in GDI and common
    /// in real files: the selection keeps working until something else is selected. Nothing here
    /// tells the device context, which is why <see cref="MetafileDeviceContext"/> holds the
    /// selected object by value rather than by handle.
    /// </remarks>
    public void Delete(int handle)
    {
        if (handle < 0 || handle >= _slots.Count) return;

        _slots[handle] = null;
        if (handle < _lowestFree) _lowestFree = handle;
    }
}
