# LibreOffice Template Library

Very basic template functionality, a bit like boost or STL, but specific to LibreOffice.

o3tl stands for "OOo [o3, get it?] template library", in which OOo stands for OpenOffice.org,
predecessor of LibreOffice.

From <https://blog.thebehrens.net/2006/01/15/update-cow_wrapper-is-available-now/>
The scope for o3tl is admittedly kind of ambitious, as it should contain "...very basic (template)
functionality, comparable to what's provided by boost or stl, but specific to OOo (what comes to mind
are e.g. STL adapters for our own data types and UNO, and stuff that could in principle be upstreamed
to boost, but isn't as of now)."

## Class Overview

- `git:o3tl/inc/o3tl/cow_wrapper.hxx`

    A copy-on-write wrapper.

- `git:o3tl/inc/o3tl/lazy_update.hxx`

    This template collects data in input type, and updates the output type with the given update functor,
    but only if the output is requested. Useful if updating is expensive, or input changes frequently, but
    output is only comparatively seldom used.

- `git:o3tl/inc/o3tl/range.hxx`

    Represents a range of integer or iterator values.

- `git:o3tl/inc/o3tl/vector_pool.hxx`

    Simple vector-based memory pool allocator.

- `git:o3tl/inc/o3tl/functional.hxx`

    Some more templates, leftovers in spirit of STLport's old functional
    header that are not part of the C++ standard (STLport has been
    replaced by direct use of the C++ STL in LibreOffice).
