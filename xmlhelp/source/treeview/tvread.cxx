#ifndef _TREEVIEW_TVREAD_HXX_
#include "tvread.hxx"
#endif

using namespace treeview;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::util;
using namespace com::sun::star::container;


TVRead::TVRead( const Reference< XMultiServiceFactory >& xMSF )
    : m_xMSF( xMSF ),
      Title( rtl::OUString::createFromAscii( "startpage" ) ),
      TargetURL( rtl::OUString::createFromAscii( "vnd.sun.star.help://swriter/start?Language=de&System=WIN" ) ),
      Children( 0 )
{
}



TVRead::TVRead( const Reference< XMultiServiceFactory >& xMSF,
                const Sequence< Any >& aSeq )
    : m_xMSF( xMSF ),
      Title( rtl::OUString::createFromAscii( "root" ) ),
      TargetURL(),
      Children( 1 )
{
    Children[0] = new TVRead( xMSF );
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
           RuntimeException)
{
    Any aAny;
    if( aName.compareToAscii( "Title" ) == 0 )
        aAny <<= Title;
    else if( aName.compareToAscii( "TargetURL" ) == 0 )
        aAny <<= TargetURL;
    else
    {
        for( unsigned int i = 0; i < Children.size(); ++i )
            if( Children[i]->Title == aName )
            {
                cppu::OWeakObject* p = Children[i].get();
                  aAny <<= Reference< XInterface >( p );
                return aAny;
            }
        throw NoSuchElementException();
    }

    return aAny;
}




Sequence< rtl::OUString > SAL_CALL
TVRead::getElementNames( )
    throw( RuntimeException )
{
    Sequence< rtl::OUString > seq( Children.size() );
    for( unsigned int i = 0; i < Children.size(); ++i )
        seq[i] = Children[i]->Title;

    return seq;
}



sal_Bool SAL_CALL
TVRead::hasByName( const rtl::OUString& aName )
    throw( RuntimeException )
{
    if( aName.compareToAscii( "Title" ) == 0        ||
        aName.compareToAscii( "TargetURL" ) == 0      )
        return true;
    else
        for( unsigned int i = 0; i < Children.size(); ++i )
            if( aName == Children[i]->Title )
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
    if( ( idx = name.indexOf( sal_Unicode( '/' ) ) ) != -1 )
    {
        rtl::OUString childName = name.copy( 0,idx );
        TVRead* p = 0;
        for( unsigned int i = 0; i < Children.size(); ++i )
            if( Children[i]->Title == childName )
            {
                p = Children[i].get();
                break;
            }

        if( p )
            return p->getByHierarchicalName( name.copy( 1 + idx ) );
        else
            throw NoSuchElementException();
    }

    return getByName( aName );
}



sal_Bool SAL_CALL
TVRead::hasByHierarchicalName( const rtl::OUString& aName )
    throw( RuntimeException )
{
    sal_Int32 idx;
    rtl::OUString name( aName );
       if( ( idx = name.indexOf( sal_Unicode( '/' ) ) ) != -1 )
    {
        rtl::OUString childName = name.copy( 0,idx );
        TVRead* p = 0;
        for( unsigned int i = 0; i < Children.size(); ++i )
            if( Children[i]->Title == childName )
            {
                p = Children[i].get();
                break;
            }

        if( p )
            return p->hasByHierarchicalName( name.copy( 1 + idx ) );
        else
            return false;
    }

    return hasByName( aName );
}
