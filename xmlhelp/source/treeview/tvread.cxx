#include <string.h>
#ifndef _TREEVIEW_TVREAD_HXX_
#include "tvread.hxx"
#endif
#ifndef XmlParse_INCLUDED
#include <expat/xmlparse.h>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif


using namespace treeview;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::util;
using namespace com::sun::star::container;




TVRead::TVRead( const Reference< XMultiServiceFactory >& xMSF,
                const rtl::OUString& aTitle,
                const rtl::OUString& aTargetURL,
                const rtl::Reference< TVChildTarget > aChildren )
    : m_xMSF( xMSF ),
      Title( aTitle ),
      TargetURL( aTargetURL ),
      Children( aChildren )
{
}



TVRead::~TVRead()
{
}



//////////////////////////////////////////////////////////////////////////
// XInterface
//////////////////////////////////////////////////////////////////////////


void SAL_CALL
TVRead::acquire(
    void )
    throw( RuntimeException )
{
  OWeakObject::acquire();
}


void SAL_CALL
TVRead::release(
              void )
  throw( RuntimeException )
{
  OWeakObject::release();
}


Any SAL_CALL
TVRead::queryInterface(
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

XTYPEPROVIDER_IMPL_5( TVRead,
                         XTypeProvider,
                      XNameAccess,
                      XHierarchicalNameAccess,
                      XChangesNotifier,
                      XComponent );




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


class TVDom
{
public:

private:
};


void start_handler(void *userData,
                   const XML_Char *name,
                   const XML_Char **atts)
{
    TVDom *tvDom = static_cast< TVDom* >( userData );
    if( strcmp( name,"tree_view" ) == 0 )
    {

    }
    else if( strcmp( name,"help_section" ) == 0 )
    {

    }
    else if( strcmp( name,"node" ) == 0 )
    {

    }
    else if( strcmp( name,"topic" ) == 0 )
    {

    }
}


void end_handler(void *userData,
                 const XML_Char *name )
{
    // TVDom *tvDom = static_cast< TVDom* >( userData );
}


void data_handler( void *userData,
                   const XML_Char *s,
                   int len)
{
    // TVDom *tvDom = static_cast< TVDom* >( userData );
}


void cdatastart_handler( void *userData )
{
    // TVDom *tvDom = static_cast< TVDom* >( userData );
}


void cdataend_handler( void *userData )
{
    // TVDom *tvDom = static_cast< TVDom* >( userData );
}



TVChildTarget::TVChildTarget( const Reference< XMultiServiceFactory >& xMSF,
                              const Sequence< Any >& aSeq )
    : TVRead( xMSF,rtl::OUString(),rtl::OUString() ),
      Elements( 2 )
{
    Elements[0] =
        new TVRead(
            xMSF,
            rtl::OUString::createFromAscii( "content1" ),
            rtl::OUString::createFromAscii( "vnd.sun.star.help://swriter/start?Language=de&System=WIN" ) );

    Elements[1] =
        new TVRead(
            xMSF,
            rtl::OUString::createFromAscii( "content2" ),
            rtl::OUString::createFromAscii( "vnd.sun.star.help://swriter/des?Language=de&System=WIN" ) );


    osl::File aFile( rtl::OUString::createFromAscii(
        "file:///e:/src637a3_49/help/de/treeview.xml" ) );

    sal_uInt64 ret,len = 7258;
    char* s = new char[ int(len) ];
    int isFinal = true;

    XML_Parser parser = XML_ParserCreate( 0 );

    aFile.open( OpenFlag_Read );
    aFile.read( s,len,ret );


    XML_SetElementHandler( parser,
                           start_handler,
                           end_handler );

    XML_SetCharacterDataHandler( parser,
                                 data_handler);


    XML_SetCdataSectionHandler( parser,
                                cdatastart_handler,
                                cdataend_handler );

    TVDom* tvDom = new TVDom();
    XML_SetUserData( parser,tvDom );


    int parsed = XML_Parse( parser,s,int( len ),isFinal );

    delete tvDom;
    aFile.close();
    delete[] s;
    XML_ParserFree( parser );
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
    sal_Int32 idx = aName.toInt32() - 1;
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
    sal_Int32 idx = aName.toInt32() - 1;
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
        sal_Int32 pref = name.copy( 0,idx ).toInt32() - 1;

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
        sal_Int32 pref = name.copy( 0,idx ).toInt32() - 1;

        if( pref < 0 || Elements.size() <= sal_uInt32( pref ) )
            return false;

        return Elements[pref]->hasByHierarchicalName( name.copy( 1 + idx ) );
    }
    else
        return hasByName( name );
}
