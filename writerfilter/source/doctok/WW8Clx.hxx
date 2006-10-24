#ifndef INCLUDED_WW8_CLX_HXX
#define INCLUDED_WW8_CLX_HXX

#ifndef INCLUDED_RESOURCES_HXX
#include <resources.hxx>
#endif

#ifndef INCLUDED_WW8_OUTPUT_WITH_DEPTH_HXX
#include <WW8OutputWithDepth.hxx>
#endif

namespace doctok
{

/**
   Piece table as stored in a Word file.

   It contains character positions (CPs) and file character positions
   (FCs). Each CP is correlated to one FC.
 */
class WW8Clx : public WW8StructBase
{
    /// Pointer to the file information block.
    WW8Fib::Pointer_t pFib;

    /// Offset where FCs begin
    sal_uInt32 nOffsetPieceTable;

public:
    /// Pointer to Clx.
    typedef boost::shared_ptr<WW8Clx> Pointer_t;

    WW8Clx(WW8Stream & rStream,
           sal_uInt32 nOffset, sal_uInt32 nCount);

    virtual void dump(WW8OutputWithDepth & o) const;

    /**
       Return the number of pieces.
    */
    sal_uInt32 getPieceCount() const;

    /**
       Return a certain character position.

       @param nIndex     index of the character position
     */
    sal_uInt32 getCp(sal_uInt32 nIndex) const;

    /**
       Return a certain file character position.

       @param nIndex     index of the file character position
     */
    sal_uInt32 getFc(sal_uInt32 nIndex) const;

    /**
       Return if a certain FC is complex.

       @nIndex           index of the FC
     */
    sal_Bool isComplexFc(sal_uInt32 nIndex) const;

};
}

#endif
