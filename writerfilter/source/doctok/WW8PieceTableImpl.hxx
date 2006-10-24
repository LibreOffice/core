#ifndef INCLUDED_WW8_PIECE_TABLE_IMPL_HXX
#define INCLUDED_WW8_PIECE_TABLE_IMPL_HXX

#include <vector>
#ifndef INCLUDED_WW8_CP_AND_FC_HXX
#include <WW8CpAndFc.hxx>
#endif
#ifndef INCLUDE_WW8_PIECE_TABLE_HXX
#include <WW8PieceTable.hxx>
#endif

namespace doctok
{
using namespace ::std;

class WW8PieceTableImpl : public WW8PieceTable
{
    typedef std::vector<CpAndFc> tEntries;
    tEntries mEntries;

    tEntries::const_iterator findCp(const Cp & rCp) const;
    tEntries::const_iterator findFc(const Fc & rFc) const;

    mutable Cp2FcHashMap_t mCp2FcCache;

public:
    WW8PieceTableImpl(WW8Stream & rStream, sal_uInt32 nOffset,
                      sal_uInt32 nCount);

    virtual Fc cp2fc(const Cp & aCpIn) const;
    virtual Cp fc2cp(const Fc & aFcIn) const;
    virtual bool isComplex(const Cp & aCp) const;
    virtual bool isComplex(const Fc & aFc) const;

    virtual sal_uInt32 getCount() const;
    virtual Cp getFirstCp() const;
    virtual Fc getFirstFc() const;
    virtual Cp getLastCp() const;
    virtual Fc getLastFc() const;
    virtual Cp getCp(sal_uInt32 nIndex) const;
    virtual Fc getFc(sal_uInt32 nIndex) const;

    virtual void dump(ostream & o) const;
};
}

#endif // INCLUDED_WW8_PIECE_TABLE_IMPL_HXX
