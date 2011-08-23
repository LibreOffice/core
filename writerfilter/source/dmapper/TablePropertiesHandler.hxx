#ifndef INCLUDED_TABLEPROPERTIESHANDLER_HXX
#define INCLUDED_TABLEPROPERTIESHANDLER_HXX

#include <PropertyMap.hxx>

#include <resourcemodel/TableManager.hxx>
#include <WriterFilterDllApi.hxx>
#include <resourcemodel/WW8ResourceModel.hxx>

#include <boost/shared_ptr.hpp>

#include <vector>

namespace writerfilter {
namespace dmapper {


typedef ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > Handle_t;
typedef TableManager<Handle_t , TablePropertyMapPtr > DomainMapperTableManager_Base_t;

class TablePropertiesHandler
{
private:
    vector< PropertyMapPtr > m_rPropertiesStack;
    PropertyMapPtr m_pCurrentProperties;
    DomainMapperTableManager_Base_t *m_pTableManager;
    bool m_bOOXML;

public:
    TablePropertiesHandler( bool bOOXML );
    virtual ~TablePropertiesHandler( );

    bool sprm(Sprm & sprm); 

    inline void SetTableManager( DomainMapperTableManager_Base_t *pTableManager )
    {
        m_pTableManager = pTableManager;
    };

    inline void SetProperties( PropertyMapPtr pProperties )
    {
        m_pCurrentProperties = pProperties;
    };

private:

    inline void cellProps( TablePropertyMapPtr pProps )
    {
        if ( m_pTableManager )
            m_pTableManager->cellProps( pProps );
        else
            m_pCurrentProperties->insert( pProps, true );
    };

    inline void cellPropsByCell( unsigned int i, TablePropertyMapPtr pProps )
    {
        if ( m_pTableManager )
            m_pTableManager->cellPropsByCell( i, pProps );
        else
            m_pCurrentProperties->insert( pProps, true );
    };

    inline void insertRowProps( TablePropertyMapPtr pProps )
    {
        if ( m_pTableManager )
            m_pTableManager->insertRowProps( pProps );
        else
            m_pCurrentProperties->insert( pProps, true );
    };

    inline void insertTableProps( TablePropertyMapPtr pProps )
    {
        if ( m_pTableManager )
            m_pTableManager->insertTableProps( pProps );
        else
            m_pCurrentProperties->insert( pProps, true );
    };
};
typedef boost::shared_ptr<TablePropertiesHandler> TablePropertiesHandlerPtr;

} }

#endif
