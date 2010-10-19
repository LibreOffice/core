#include "grouptable.hxx"

#include <tools/gen.hxx>

using ::com::sun::star::uno::Reference;
using ::com::sun::star::container::XIndexAccess;

// ---------------------------------------------------------------------------------------------

GroupTable::GroupTable() :
    mnCurrentGroupEntry ( 0 ),
    mnMaxGroupEntry     ( 0 ),
    mnGroupsClosed      ( 0 ),
    mpGroupEntry        ( NULL )
{
    ImplResizeGroupTable( 32 );
}

// ---------------------------------------------------------------------------------------------

GroupTable::~GroupTable()
{
    for ( sal_uInt32 i = 0; i < mnCurrentGroupEntry; delete mpGroupEntry[ i++ ] ) {}
    delete[] mpGroupEntry;
}

// ---------------------------------------------------------------------------------------------

void GroupTable::ImplResizeGroupTable( sal_uInt32 nEntrys )
{
    if ( nEntrys > mnMaxGroupEntry )
    {
        mnMaxGroupEntry         = nEntrys;
        GroupEntry** pTemp = new GroupEntry*[ nEntrys ];
        for ( sal_uInt32 i = 0; i < mnCurrentGroupEntry; i++ )
            pTemp[ i ] = mpGroupEntry[ i ];
        if ( mpGroupEntry )
            delete[] mpGroupEntry;
        mpGroupEntry = pTemp;
    }
}

// ---------------------------------------------------------------------------------------------

sal_Bool GroupTable::EnterGroup( Reference< XIndexAccess >& rXIndexAccessRef )
{
    sal_Bool bRet = sal_False;
    if ( rXIndexAccessRef.is() )
    {
        GroupEntry* pNewGroup = new GroupEntry( rXIndexAccessRef );
        if ( pNewGroup->mnCount )
        {
            if ( mnMaxGroupEntry == mnCurrentGroupEntry )
                ImplResizeGroupTable( mnMaxGroupEntry + 8 );
            mpGroupEntry[ mnCurrentGroupEntry++ ] = pNewGroup;
            bRet = sal_True;
        }
        else
            delete pNewGroup;
    }
    return bRet;
}

// ---------------------------------------------------------------------------------------------

sal_uInt32 GroupTable::GetGroupsClosed()
{
    sal_uInt32 nRet = mnGroupsClosed;
    mnGroupsClosed = 0;
    return nRet;
}

// ---------------------------------------------------------------------------------------------

void GroupTable::ClearGroupTable()
{
    for ( sal_uInt32 i = 0; i < mnCurrentGroupEntry; i++, delete mpGroupEntry[ i ] ) {}
    mnCurrentGroupEntry = 0;
}

// ---------------------------------------------------------------------------------------------

void GroupTable::ResetGroupTable( sal_uInt32 nCount )
{
    ClearGroupTable();
    mpGroupEntry[ mnCurrentGroupEntry++ ] = new GroupEntry( nCount );
}

// ---------------------------------------------------------------------------------------------

sal_Bool GroupTable::GetNextGroupEntry()
{
    while ( mnCurrentGroupEntry )
    {
        mnIndex = mpGroupEntry[ mnCurrentGroupEntry - 1 ]->mnCurrentPos++;

        if ( mpGroupEntry[ mnCurrentGroupEntry - 1 ]->mnCount > mnIndex )
            return TRUE;

        delete ( mpGroupEntry[ --mnCurrentGroupEntry ] );

        if ( mnCurrentGroupEntry )
            mnGroupsClosed++;
    }
    return FALSE;
}

// ---------------------------------------------------------------------------------------------

void GroupTable::SkipCurrentGroup()
{
    if ( mnCurrentGroupEntry )
        delete ( mpGroupEntry[ --mnCurrentGroupEntry ] );
}
