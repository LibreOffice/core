#include "SettingsTable.hxx"

#include <ooxml/resourceids.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XTextDocument.hpp>

#if DEBUG
#include <iostream>
#endif

using namespace com::sun::star;
using namespace rtl;

namespace writerfilter {
namespace dmapper {

SettingsTable_Impl::SettingsTable_Impl( ) :
   m_bRecordChanges( false )
{
}

SettingsTable_Impl::~SettingsTable_Impl( )
{
}

SettingsTable::SettingsTable( ) :
    m_pImpl( new SettingsTable_Impl )
{
}

SettingsTable::~SettingsTable( )
{
    delete m_pImpl, m_pImpl = NULL;
}

void SettingsTable::attribute( Id /*nName*/, Value& /*rVal*/ )
{
#if DEBUG
    clog << "SettingsTable::attribute( )" << endl;
#endif
}

void SettingsTable::sprm( Sprm& rSprm )
{
    switch ( rSprm.getId( ) )
    {
        case NS_ooxml::LN_CT_Settings_trackRevisions:
            {
                m_pImpl->m_bRecordChanges = bool(rSprm.getValue( )->getInt( ) );
            }
            break;
        default:
            {
#if DEBUG
                clog << __FILE__ << ":" << __LINE__;
                clog << ": unknown SPRM: " << rSprm.getName( ) << endl;
#endif
            }
    }
}

void SettingsTable::entry( int nPos, writerfilter::Reference<Properties>::Pointer_t pRef )
{
    (void)nPos;
#if DEBUG
    fprintf( stderr, "SettingsTable::entry( ), pos: %d\n", nPos );
#endif
    pRef->resolve( *this );
}

void SettingsTable::resolveSprmProps( Sprm& rSprm )
{
    writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
    if( pProperties.get())
        pProperties->resolve(*this);
}

void SettingsTable::ApplyProperties( uno::Reference< text::XTextDocument > xDoc )
{
    uno::Reference< beans::XPropertySet> xDocProps( xDoc, uno::UNO_QUERY );

    // Record changes value
    xDocProps->setPropertyValue( OUString::createFromAscii( "RecordChanges" ), uno::makeAny( m_pImpl->m_bRecordChanges ) );
}

} }
