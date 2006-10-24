#ifndef INCLUDED_WW8_BIN_TABLE_HXX
#define INCLUDED_WW8_BIN_TABLE_HXX

#include <boost/shared_ptr.hpp>
#include <string>

#ifndef INCLUDED_WW8_FKP_HXX
#include <WW8FKP.hxx>
#endif

namespace doctok
{

/**
   A bintable.

   Word uses bintables to associate FC ranges with FKPs. A bintable
   has a list of FCs. At each FC a range begins. The length of the
   range is given by the distance of the according CPs.
 */
class WW8BinTable
{
public:
    virtual ~WW8BinTable() {};
    /**
       Shared pointer to a bintable.
     */
    typedef boost::shared_ptr<WW8BinTable> Pointer_t;

    /**
       Return count of entries.
     */
    virtual sal_uInt32 getEntryCount() const = 0;

    /**
       Return FC from bintable.

       @param nIndex    index in bintable to return FC from
     */
    virtual Fc getFc(sal_uInt32 nIndex) const = 0;

    /**
       Return page number.

       @param nIndex    index in bintable to return page number from
     */
    virtual sal_uInt32 getPageNumber(sal_uInt32 nIndex) const = 0;

    /**
       Return page number associated with FC.

       @param rFc      FC to return page number for
     */
    virtual sal_uInt32 getPageNumber(const Fc & rFc) const = 0;

    /**
       Return string representation of bintable.
     */
    virtual string toString() const = 0;
};
}

#endif // INCLUDED_WW8_BIN_TABLE_HXX
