#ifndef _UOF2_SPLITER_HXX_
#define _UOF2_SPLITER_HXX_

#include <com/sun/star/uno/Reference.hxx>

#include "uof2splithandler.hxx"
#include <com/sun/star/io/XActiveDataControl.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XActiveDataStreamer.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <cppuhelper/implbase3.hxx>
#include <osl/mutex.hxx>
#include <osl/thread.h>

#include <set>

namespace com{
    namespace sun{
        namespace star{
            namespace io{
                class XInputStream;
                class XStream;
                class XStreamListener;
                class XActiveDataStreamer;
            }
            namespace lang{
                //class XInitialization;
                class XMultiServiceFactory;
            }
        }
    }
}


namespace XSLT{

class UOF2Splitter : public ::cppu::WeakImplHelper3<
    ::com::sun::star::io::XActiveDataControl ,
    ::com::sun::star::io::XActiveDataStreamer ,
    ::com::sun::star::io::XActiveDataSink >//,
    //::com::sun::star::lang::XInitialization >
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_rMultiFactory;
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XExtendedDocumentHandler >  m_xWriter;
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > m_xStream;
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > m_xInputStream;
    std::set< ::com::sun::star::uno::Reference< ::com::sun::star::io::XStreamListener > > m_StreamListeners;
    ::rtl::OUString aURL;
    ::osl::Mutex    m_aMutex;
    oslThread    m_aThread;

public :
    UOF2Splitter( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rFactory , const ::rtl::OUString & rURL );
    ~UOF2Splitter();

    // XActiveDataControl
    virtual void SAL_CALL   addListener ( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStreamListener> & rxListener ) throw( ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeListener ( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStreamListener> & rxListener ) throw( ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   start (void) throw( ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   terminate (void) throw( ::com::sun::star::uno::RuntimeException);

    // XActiveDataSink
    virtual void SAL_CALL   setInputStream ( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream> &rxInputStream) throw( ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream> SAL_CALL getInputStream (void) throw( ::com::sun::star::uno::RuntimeException);

    // XActiveDataStreamer
    virtual void SAL_CALL   setStream ( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream> &rxStream) throw( ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream> SAL_CALL getStream (void) throw( ::com::sun::star::uno::RuntimeException);

};

}
#endif
