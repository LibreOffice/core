#ifndef INCLUDED_WW8_BIN_TABLE_IMPL_HXX
#define INCLUDED_WW8_BIN_TABLE_IMPL_HXX

#include <istream>
#ifndef INCLUDED_WW8_BIN_TABLE_HXX
#include <WW8BinTable.hxx>
#endif
#ifndef INCLUDED_PLCF_HXX
#include <PLCF.hxx>
#endif

#include <hash_map>

namespace doctok
{
using namespace ::std;

/**
   A number of a FKP.
 */
class PageNumber
{
    /// the page number
    sal_uInt32 mnPageNumber;

public:
    /// Pointer to a page number
    typedef boost::shared_ptr<PageNumber> Pointer_t;

    /// get size of a page number
    static size_t getSize() { return 4; }

    PageNumber(WW8StructBase::Sequence & rSeq, sal_uInt32 nOffset,
               sal_uInt32 /*nCount*/)
    : mnPageNumber(getU32(rSeq, nOffset))
    {
    }

    /// Return the page number
    sal_uInt32 get() const { return mnPageNumber; }

    virtual void dump(OutputWithDepth<string> & output) const;
};

/**
   Implementation class for a binary table
 */
class WW8BinTableImpl : public WW8BinTable
{
    /// PLCF containing the numbers of the FKPs of the binary table
    PLCF<PageNumber> mData;
    mutable hash_map<Fc, sal_uInt32, FcHash> mPageMap;

public:
    WW8BinTableImpl(WW8Stream & rStream, sal_uInt32 nOffset,
                    sal_uInt32 nCount)
    : mData(rStream, nOffset, nCount)

    {
    }

    virtual sal_uInt32 getEntryCount() const
    { return mData.getEntryCount(); }
    virtual Fc getFc(sal_uInt32 nIndex) const
    { return mData.getFc(nIndex); }
    virtual sal_uInt32 getPageNumber(sal_uInt32 nIndex) const
    { return mData.getEntry(nIndex)->get(); }
    virtual sal_uInt32 getPageNumber(const Fc & rFc) const;
    virtual string toString() const;
};

}

#endif // INCLUDED_WW8_BIN_TABLE_IMPL_HXX
