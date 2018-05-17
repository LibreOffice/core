
#ifndef INCLUDED_SW_SOURCE_CORE_INC_TEXTFRAMEINDEX_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_TEXTFRAMEINDEX_HXX

#include <sal/types.h>
#include <o3tl/strong_int.hxx>

struct Tag_TextFrameIndex
{
};
#if 0
typedef o3tl::strong_int<sal_Int32, Tag_TextFrameIndex> TextFrameIndex;
#else
typedef sal_Int32 TextFrameIndex;
#endif

#endif // INCLUDED_SW_SOURCE_CORE_INC_TEXTFRAMEINDEX_HXX
