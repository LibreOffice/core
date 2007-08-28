#ifndef _TOOLS_WEAKBASE_HXX_
#define _TOOLS_WEAKBASE_HXX_

#ifndef _TOOLS_WEAKBASE_H_
#include <tools/weakbase.h>
#endif

/// see weakbase.h for documentation

namespace tools
{

template< class reference_type >
inline WeakReference< reference_type >::WeakReference()
{
    mpWeakConnection = new WeakConnection<reference_type>( 0 );
    mpWeakConnection->acquire();
}

template< class reference_type >
inline WeakReference< reference_type >::WeakReference( reference_type* pReference )
{
    if( pReference )
        mpWeakConnection = pReference->getWeakConnection();
    else
        mpWeakConnection = new WeakConnection<reference_type>( 0 );

    mpWeakConnection->acquire();
}

template< class reference_type >
inline WeakReference< reference_type >::WeakReference( const WeakReference< reference_type >& rWeakRef )
{
    mpWeakConnection = rWeakRef.mpWeakConnection;
    mpWeakConnection->acquire();
}

template< class reference_type >
inline WeakReference< reference_type >::~WeakReference()
{
    mpWeakConnection->release();
}

template< class reference_type >
inline bool WeakReference< reference_type >::is() const
{
    return mpWeakConnection->mpReference != 0;
}

template< class reference_type >
inline reference_type * WeakReference< reference_type >::get() const
{
    return mpWeakConnection->mpReference;
}

template< class reference_type >
inline void WeakReference< reference_type >::reset( reference_type* pReference )
{
    mpWeakConnection->release();

    if( pReference )
        mpWeakConnection = pReference->getWeakConnection();
    else
        mpWeakConnection = new WeakConnection<reference_type>( 0 );

    mpWeakConnection->acquire();
}

template< class reference_type >
inline reference_type * WeakReference< reference_type >::operator->() const
{
    OSL_PRECOND(mpWeakConnection, "tools::WeakReference::operator->() : null body");
    return mpWeakConnection->mpReference;
}

template< class reference_type >
inline sal_Bool WeakReference< reference_type >::operator==(const reference_type * pReferenceObject) const
{
    return mpWeakConnection->mpReference == pReferenceObject;
}

template< class reference_type >
inline sal_Bool WeakReference< reference_type >::operator==(const WeakReference<reference_type> & handle) const
{
    return mpWeakConnection == handle.mpWeakConnection;
}

template< class reference_type >
inline sal_Bool WeakReference< reference_type >::operator!=(const WeakReference<reference_type> & handle) const
{
    return mpWeakConnection != handle.mpWeakConnection;
}

template< class reference_type >
inline sal_Bool WeakReference< reference_type >::operator<(const WeakReference<reference_type> & handle) const
{
    return mpWeakConnection->mpReference < handle.mpWeakConnection->mpReference;
}

template< class reference_type >
inline sal_Bool WeakReference< reference_type >::operator>(const WeakReference<reference_type> & handle) const
{
    return mpWeakConnection->mpReference > handle.mpWeakConnection->mpReference;
}

template< class reference_type >
inline WeakReference<reference_type>& WeakReference<reference_type>::operator= (
    const WeakReference<reference_type>& rReference)
{
    if (&rReference != this)
    {
        mpWeakConnection->release();

        mpWeakConnection = rReference.mpWeakConnection;
        mpWeakConnection->acquire();
    }
    return *this;
}

template< class reference_type >
inline WeakBase< reference_type >::WeakBase()
{
    mpWeakConnection = 0;
}

template< class reference_type >
inline WeakBase< reference_type >::~WeakBase()
{
    if( mpWeakConnection )
    {
        mpWeakConnection->mpReference = 0;
        mpWeakConnection->release();
        mpWeakConnection = 0;
    }
}

template< class reference_type >
inline void WeakBase< reference_type >::clearWeak()
{
    if( mpWeakConnection )
        mpWeakConnection->mpReference = 0;
}

template< class reference_type >
inline WeakConnection< reference_type >* WeakBase< reference_type >::getWeakConnection()
{
    if( !mpWeakConnection )
    {
        mpWeakConnection = new WeakConnection< reference_type >( static_cast< reference_type* >( this ) );
        mpWeakConnection->acquire();
    }
    return mpWeakConnection;
}

}

#endif // _TOOLS_WEAKBASE_HXX_

