#include <string.h>
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _VOS_DIAGNOSE_HXX_
#include <vos/diagnose.hxx>
#endif
#ifndef _TREEVIEW_TVREAD_HXX_
#include "tvread.hxx"
#endif
#ifndef XmlParse_INCLUDED
#include <expat/xmlparse.h>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONFIGMANAGER_HPP_
#include <com/sun/star/frame/XConfigManager.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATE_HPP_
#include <com/sun/star/beans/PropertyState.hpp>
#endif

namespace treeview {


    class TVDom
    {
        friend class TVChildTarget;
        friend class TVRead;

    public:

        TVDom( TVDom* arent = 0 )
            : parent( arent ),
              childs( 0 ),
              kind( other )
        {
        }

        ~TVDom()
        {
            for( unsigned i = 0; i < childs.size(); ++i )
                delete childs[i];
        }


        TVDom* newChild()
        {
            childs.push_back( new TVDom( this ) );
            return childs.back();
        }


        TVDom* getParent() const
        {
            return parent;
        }

        enum Kind {
            tree_view,
            tree_node,
            tree_leaf,
            other
        };

        bool isLeaf() const { return kind == TVDom::tree_leaf; }
        void setKind( Kind ind ) { kind = ind; }
        Kind getKind( ) const { return kind; }


        void setApplication( const char* appl )
        {
            application = rtl::OUString( (sal_Char*)(appl),
                                         strlen( appl ),
                                         RTL_TEXTENCODING_UTF8 );
        }

        void setTitle( const char* itle )
        {
            title = rtl::OUString( (sal_Char*)(itle),
                                   strlen( itle ),
                                   RTL_TEXTENCODING_UTF8 );
        }

        void setTitle( const XML_Char* itle,int len )
        {
            title = rtl::OUString( (sal_Char*)(itle),
                                   len,
                                   RTL_TEXTENCODING_UTF8 );
        }

        void setId( const char* d )
        {
            id = rtl::OUString( (sal_Char*)(d),
                                strlen( d ),
                                RTL_TEXTENCODING_UTF8 );
        }

        void setAnchor( const char* nchor )
        {
            anchor = rtl::OUString( (sal_Char*)(nchor),
                                    strlen( nchor ),
                                    RTL_TEXTENCODING_UTF8 );
        }

        rtl::OUString getTargetURL()
        {
            if( ! targetURL.getLength() )
            {
                const TVDom* p = this;
                sal_Int32 len;
                while( ! ( len = p->application.getLength() ) )
                    p = p->parent;

                rtl::OUStringBuffer strBuff( 22 + len + id.getLength() );
                strBuff.appendAscii(
                    "vnd.sun.star.help://"
                ).append(
                    p->application
                ).appendAscii(
                    "/"
                ).append(
                    id
                );

                targetURL = strBuff.makeStringAndClear();
            }

            return targetURL;
        }

    private:

        Kind   kind;
        rtl::OUString  application;
        rtl::OUString  title;
        rtl::OUString  id;
        rtl::OUString  anchor;
        rtl::OUString  targetURL;

        TVDom *parent;
        std::vector< TVDom* > childs;
    };

}



using namespace treeview;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::util;
using namespace com::sun::star::frame;
using namespace com::sun::star::container;




//////////////////////////////////////////////////////////////////////////
// XInterface
//////////////////////////////////////////////////////////////////////////


void SAL_CALL
TVBase::acquire(
    void )
    throw( RuntimeException )
{
  OWeakObject::acquire();
}


void SAL_CALL
TVBase::release(
              void )
  throw( RuntimeException )
{
  OWeakObject::release();
}


Any SAL_CALL
TVBase::queryInterface(
    const Type& rType )
    throw( RuntimeException )
{
    Any aRet = cppu::queryInterface( rType,
                                     SAL_STATIC_CAST( XTypeProvider*, this ),
                                     SAL_STATIC_CAST( XNameAccess*, this ),
                                     SAL_STATIC_CAST( XHierarchicalNameAccess*, this ),
                                     SAL_STATIC_CAST( XChangesNotifier*, this ),
                                     SAL_STATIC_CAST( XComponent*, this ) );

    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


////////////////////////////////////////////////////////////////////////////////
//
// XTypeProvider methods.

XTYPEPROVIDER_IMPL_5( TVBase,
                         XTypeProvider,
                      XNameAccess,
                      XHierarchicalNameAccess,
                      XChangesNotifier,
                      XComponent );






// TVRead


TVRead::TVRead()
{
}



TVRead::TVRead( const ConfigData& configData,TVDom* tvDom )
{
    if( ! tvDom )
        return;

    Title = tvDom->title;
    if( tvDom->isLeaf() )
    {
        TargetURL = ( tvDom->getTargetURL() + configData.appendix );
        if( tvDom->anchor.getLength() )
            TargetURL += ( rtl::OUString::createFromAscii( "#" ) +
                           tvDom->anchor );
    }
    else
        Children = new TVChildTarget( configData,tvDom );
}



TVRead::~TVRead()
{
}






// XNameAccess

Any SAL_CALL
TVRead::getByName( const rtl::OUString& aName )
    throw( NoSuchElementException,
           WrappedTargetException,
           RuntimeException )
{
    bool found( true );
    Any aAny;
    if( aName.compareToAscii( "Title" ) == 0 )
        aAny <<= Title;
    else if( aName.compareToAscii( "TargetURL" ) == 0 )
        aAny <<= TargetURL;
    else if( aName.compareToAscii( "Children" ) == 0 )
    {
        cppu::OWeakObject* p = Children.get();
        aAny <<= Reference< XInterface >( p );
    }
    else
        found = false;

    if( found )
        return aAny;

    throw NoSuchElementException();
}




Sequence< rtl::OUString > SAL_CALL
TVRead::getElementNames( )
    throw( RuntimeException )
{
    Sequence< rtl::OUString > seq( 3 );

    seq[0] = rtl::OUString::createFromAscii( "Title" );
    seq[1] = rtl::OUString::createFromAscii( "TargetURL" );
    seq[2] = rtl::OUString::createFromAscii( "Children" );

    return seq;
}



sal_Bool SAL_CALL
TVRead::hasByName( const rtl::OUString& aName )
    throw( RuntimeException )
{
    if( aName.compareToAscii( "Title" ) == 0        ||
        aName.compareToAscii( "TargetURL" ) == 0    ||
        aName.compareToAscii( "Children" ) == 0 )
        return true;

    return false;
}


// XHierarchicalNameAccess

Any SAL_CALL
TVRead::getByHierarchicalName( const rtl::OUString& aName )
    throw( NoSuchElementException,
           RuntimeException )
{
    sal_Int32 idx;
    rtl::OUString name( aName );

    if( ( idx = name.indexOf( sal_Unicode( '/' ) ) ) != -1  &&
        name.copy( 0,idx ).compareToAscii( "Children" ) == 0 )
        return Children->getByHierarchicalName( name.copy( 1 + idx ) );

    return getByName( name );
}




sal_Bool SAL_CALL
TVRead::hasByHierarchicalName( const rtl::OUString& aName )
    throw( RuntimeException )
{
    sal_Int32 idx;
    rtl::OUString name( aName );

       if( ( idx = name.indexOf( sal_Unicode( '/' ) ) ) != -1  &&
        name.copy( 0,idx ).compareToAscii( "Children" ) == 0 )
        return Children->hasByHierarchicalName( name.copy( 1 + idx ) );

    return hasByName( name );
}



/**************************************************************************/
/*                                                                        */
/*                      TVChildTarget                                     */
/*                                                                        */
/**************************************************************************/




void start_handler(void *userData,
                   const XML_Char *name,
                   const XML_Char **atts)
{
    TVDom::Kind kind;

    if( strcmp( name,"help_section" ) == 0  ||
        strcmp( name,"node" ) == 0 )
        kind = TVDom::tree_node;
    else if( strcmp( name,"topic" ) == 0 )
        kind = TVDom::tree_leaf;
    else
        return;

    TVDom **tvDom = static_cast< TVDom** >( userData );
    TVDom  *p;
    p = *tvDom;

    *tvDom = p->newChild();
    p = *tvDom;

    p->setKind( kind );
    while( *atts )
    {
        if( strcmp( *atts,"application" ) == 0 )
            p->setApplication( *(atts+1) );
        else if( strcmp( *atts,"title" ) == 0 )
            p->setTitle( *(atts+1) );
        else if( strcmp( *atts,"id" ) == 0 )
            p->setId( *(atts+1) );
        else if( strcmp( *atts,"anchor" ) == 0 )
            p->setAnchor( *(atts+1) );

        atts+=2;
    }
}


void end_handler(void *userData,
                 const XML_Char *name )
{
    TVDom **tvDom = static_cast< TVDom** >( userData );
    *tvDom = (*tvDom)->getParent();
}


void data_handler( void *userData,
                   const XML_Char *s,
                   int len)
{
    TVDom **tvDom = static_cast< TVDom** >( userData );
    if( (*tvDom)->isLeaf() )
        (*tvDom)->setTitle( s,len );
}



TVChildTarget::TVChildTarget( const ConfigData& configData,TVDom* tvDom )
{
    Elements.resize( tvDom->childs.size() );
    for( unsigned i = 0; i < Elements.size(); ++i )
        Elements[i] = new TVRead( configData,tvDom->childs[i] );
}





TVChildTarget::TVChildTarget( const Reference< XMultiServiceFactory >& xMSF )
{
    ConfigData configData = init( xMSF );

    if( ! configData.fileurl.getLength() ||
        ! configData.locale.getLength()  ||
        ! configData.system.getLength()  ||
        ! configData.filelen )
        return;

    osl::File aFile(
        configData.fileurl );

    configData.appendix =
        rtl::OUString::createFromAscii( "?Language=" ) +
        configData.locale +
        rtl::OUString::createFromAscii( "&System=" ) +
        configData.system;

    sal_uInt64 ret,len = configData.filelen;
    if( ! len ) return;

    char* s = new char[ int(len) ];
    int isFinal = true;

    XML_Parser parser = XML_ParserCreate( 0 );

    aFile.open( OpenFlag_Read );
    aFile.read( s,len,ret );
    aFile.close();

    XML_SetElementHandler( parser,
                           start_handler,
                           end_handler );

    XML_SetCharacterDataHandler( parser,
                                 data_handler);


    TVDom tvDom;
    TVDom* pTVDom = &tvDom;
    XML_SetUserData( parser,&pTVDom );

    int parsed = XML_Parse( parser,s,int( len ),isFinal );

    delete[] s;
    XML_ParserFree( parser );

    // now TVDom holds the relevant information

    Elements.resize( tvDom.childs.size() );
    for( unsigned i = 0; i < Elements.size(); ++i )
        Elements[i] = new TVRead( configData,tvDom.childs[i] );
}


TVChildTarget::~TVChildTarget()
{
}



Any SAL_CALL
TVChildTarget::getByName( const rtl::OUString& aName )
    throw( NoSuchElementException,
           WrappedTargetException,
           RuntimeException )
{
    rtl::OUString num( aName.getStr()+2,aName.getLength()-4 );
    sal_Int32 idx = num.toInt32() - 1;
    if( idx < 0 || Elements.size() <= sal_uInt32( idx ) )
        throw NoSuchElementException();

    Any aAny;
    cppu::OWeakObject* p = Elements[idx].get();
    aAny <<= Reference< XInterface >( p );
    return aAny;
}




Sequence< rtl::OUString > SAL_CALL
TVChildTarget::getElementNames( )
    throw( RuntimeException )
{
    Sequence< rtl::OUString > seq( Elements.size() );
    for( unsigned i = 0; i < Elements.size(); ++i )
        seq[i] = rtl::OUString::valueOf( sal_Int32( 1+i ) );

    return seq;
}



sal_Bool SAL_CALL
TVChildTarget::hasByName( const rtl::OUString& aName )
    throw( RuntimeException )
{
    rtl::OUString num( aName.getStr()+2,aName.getLength()-4 );
    sal_Int32 idx = num.toInt32() - 1;
    if( idx < 0 || Elements.size() <= sal_uInt32( idx ) )
        return false;

    return true;
}



// XHierarchicalNameAccess

Any SAL_CALL
TVChildTarget::getByHierarchicalName( const rtl::OUString& aName )
    throw( NoSuchElementException,
           RuntimeException )
{
    sal_Int32 idx;
    rtl::OUString name( aName );

    if( ( idx = name.indexOf( sal_Unicode( '/' ) ) ) != -1 )
    {
        rtl::OUString num( name.getStr()+2,idx-4 );
        sal_Int32 pref = num.toInt32() - 1;

        if( pref < 0 || Elements.size() <= sal_uInt32( pref ) )
            throw NoSuchElementException();

        return Elements[pref]->getByHierarchicalName( name.copy( 1 + idx ) );
    }
    else
        return getByName( name );
}



sal_Bool SAL_CALL
TVChildTarget::hasByHierarchicalName( const rtl::OUString& aName )
    throw( RuntimeException )
{
    sal_Int32 idx;
    rtl::OUString name( aName );

       if( ( idx = name.indexOf( sal_Unicode( '/' ) ) ) != -1 )
    {
        rtl::OUString num( name.getStr()+2,idx-4 );
        sal_Int32 pref = num.toInt32() - 1;
        if( pref < 0 || Elements.size() <= sal_uInt32( pref ) )
            return false;

        return Elements[pref]->hasByHierarchicalName( name.copy( 1 + idx ) );
    }
    else
        return hasByName( name );
}






ConfigData TVChildTarget::init( const Reference< XMultiServiceFactory >& xSMgr )
{
    ConfigData configData;

    rtl::OUString sProviderService =
        rtl::OUString::createFromAscii( "com.sun.star.configuration.ConfigurationProvider" );

    Any aAny;
    aAny <<= rtl::OUString::createFromAscii( "local" );
    PropertyValue aProp( rtl::OUString::createFromAscii( "servertype" ),
                         -1,
                         aAny,
                         PropertyState_DIRECT_VALUE );

    Sequence< Any > seq(1);
    seq[0] <<= aProp;

    Reference< XMultiServiceFactory > sProvider;
    try
    {
        sProvider =
            Reference< XMultiServiceFactory >(
                xSMgr->createInstanceWithArguments( sProviderService,seq ),
                UNO_QUERY );
    }
    catch( const com::sun::star::uno::Exception& )
    {
        VOS_ENSHURE( sProvider.is()," cant instantiate the multiservicefactory " );
    }


    if( ! sProvider.is() )
        return configData;

    rtl::OUString sReaderService =
        rtl::OUString::createFromAscii( "com.sun.star.configuration.ConfigurationAccess" );

    seq[0] <<= rtl::OUString::createFromAscii( "org.openoffice.Office.Common" );


    Reference< XHierarchicalNameAccess > xHierAccess;
    try
    {
        xHierAccess =
            Reference< XHierarchicalNameAccess >
            ( sProvider->createInstanceWithArguments( sReaderService,seq ),
              UNO_QUERY );
    }
    catch( const com::sun::star::uno::Exception& )
    {
        VOS_ENSHURE( xHierAccess.is()," cant instantiate the reader service " );
    }

    if( ! xHierAccess.is() )
        return configData;

    try
    {
        aAny =
            xHierAccess->getByHierarchicalName( rtl::OUString::createFromAscii("Path/Current/Help") );
    }
    catch( const com::sun::star::container::NoSuchElementException& )
    {
        VOS_ENSHURE( false," path to help files could not be determined " );
        return configData;
    }


    rtl::OUString instPath;
    bool err = ! ( aAny >>= instPath );

    if( err )
    {
        VOS_ENSHURE( false," path to help files could not be determined " );
        return configData;
    }

    Reference< XConfigManager >  xCfgMgr;
    try
    {
        xCfgMgr =
            Reference< XConfigManager >(
                xSMgr->createInstance( rtl::OUString::createFromAscii( "com.sun.star.config.SpecialConfigManager" ) ),
                UNO_QUERY );
    }
    catch( const com::sun::star::uno::Exception& )
    {
        VOS_ENSHURE( xCfgMgr.is()," cant instantiate the special config manager " );
    }


    if( ! xCfgMgr.is() )
        return configData;

    instPath = xCfgMgr->substituteVariables( instPath );

    rtl::OUString url;
    osl::FileBase::RC errFile = osl::FileBase::getFileURLFromSystemPath( instPath,url );
    if( errFile != osl::FileBase::E_None )
        return configData;


    Any aAny1;
    try
    {
        aAny1 =
            xHierAccess->getByHierarchicalName( rtl::OUString::createFromAscii("Help/System") );
    }
    catch( const com::sun::star::container::NoSuchElementException& )
    {
        VOS_ENSHURE( false," " );
        return configData;
    }


    rtl::OUString system;
    err = ! ( aAny1 >>= system );

    if( err )
    {
        VOS_ENSHURE( false," path to help files could not be determined " );
        return configData;
    }


    // Reading Locale

    seq[0] <<= rtl::OUString::createFromAscii( "org.openoffice.Setup" );

    try
    {
        xHierAccess =
            Reference< XHierarchicalNameAccess >
            ( sProvider->createInstanceWithArguments( sReaderService,seq ),
              UNO_QUERY );
    }
    catch( const com::sun::star::uno::Exception& )
    {
        VOS_ENSHURE( xHierAccess.is()," cant instantiate the reader service " );
    }

    if( ! xHierAccess.is() )
        return configData;

    Any aAny2;
    try
    {
        aAny2 =
            xHierAccess->getByHierarchicalName( rtl::OUString::createFromAscii("L10N/ooLocale") );
    }
    catch( const com::sun::star::container::NoSuchElementException& )
    {
        VOS_ENSHURE( false," path to help files could not be determined " );
        return configData;
    }

    rtl::OUString locale;
    err = ! ( aAny2 >>= locale );

    if( err )
    {
        VOS_ENSHURE( false," ");
        return configData;
    }


    // Determine fileurl from url and locale

    if( url.lastIndexOf( sal_Unicode( '/' ) ) != url.getLength() - 1 )
        url += rtl::OUString::createFromAscii( "/" );

    rtl::OUString ret;

    sal_Int32 idx;
    osl::DirectoryItem aDirItem;

    if( osl::FileBase::E_None == osl::DirectoryItem::get( url + locale,aDirItem ) )
        ret = locale;
    else if( ( ( idx = locale.indexOf( '-' ) ) != -1 ||
               ( idx = locale.indexOf( '_' ) ) != -1 ) &&
             osl::FileBase::E_None == osl::DirectoryItem::get( url + locale.copy( 0,idx ),
                                                               aDirItem ) )
        ret = locale.copy( 0,idx );

    url = url + ret + rtl::OUString::createFromAscii( "/treeview.xml" );

    // Determine the filelen
    sal_uInt64 filelen = 0;

    osl::FileStatus aStatus( FileStatusMask_FileSize );
    if( osl::FileBase::E_None == osl::DirectoryItem::get( url,aDirItem ) &&
        osl::FileBase::E_None == aDirItem.getFileStatus( aStatus )  &&
        aStatus.isValid( FileStatusMask_FileSize ) )
        filelen = aStatus.getFileSize();


    configData.fileurl = url;
    configData.system = system;
    configData.locale = locale;
    configData.filelen = filelen;
    return configData;
}
