/*************************************************************************
 *
 *  $RCSfile: model.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:16:51 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef __PLUGIN_MODEL_HXX
#define __PLUGIN_MODEL_HXX

#ifndef _COM_SUN_STAR_IO_XOBJECTINPUTSTREAM_HPP_
#include <com/sun/star/io/XObjectInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XPERSISTOBJECT_HPP_
#include <com/sun/star/io/XPersistObject.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOBJECTOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XObjectOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICENAME_HPP_
#include <com/sun/star/lang/XServiceName.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROLMODEL_HPP_
#include <com/sun/star/awt/XControlModel.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROL_HPP_
#include <com/sun/star/awt/XControl.hpp>
#endif

#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/weakagg.hxx>
#include <rtl/alloc.h>

#include <stl/list>

using namespace com::sun::star::uno;

class BroadcasterHelperHolder
{
protected:
    ::cppu::OBroadcastHelper  m_aHelper;
public:
    BroadcasterHelperHolder( ::osl::Mutex& rMutex ) :
            m_aHelper( rMutex ) {}
    ~BroadcasterHelperHolder() {}

    ::cppu::OBroadcastHelper& getHelper() { return m_aHelper; }

};

class PluginModel : public BroadcasterHelperHolder,
                    public ::cppu::OPropertySetHelper,
                    public ::cppu::OPropertyArrayHelper,
                    public ::cppu::OWeakAggObject,
                    public ::com::sun::star::lang::XComponent,
                    public ::com::sun::star::io::XPersistObject,
                    public ::com::sun::star::awt::XControlModel
{
  private:
    ::rtl::OUString m_aCreationURL;

    ::std::list< Reference< ::com::sun::star::lang::XEventListener > >
                 m_aDisposeListeners;
  public:
    // these are here to force memory de/allocation to sal lib.
    static void * SAL_CALL operator new( size_t nSize ) throw()
        { return ::rtl_allocateMemory( nSize ); }
    static void SAL_CALL operator delete( void * pMem ) throw()
        { ::rtl_freeMemory( pMem ); }

    PluginModel();
    PluginModel( const ::rtl::OUString& );
    virtual ~PluginModel();


    const ::rtl::OUString& getCreationURL() { return m_aCreationURL; }

    // XInterface
    virtual Any SAL_CALL queryInterface( const Type& rType )
        { return OWeakAggObject::queryInterface( rType ); }
    virtual void SAL_CALL acquire() { OWeakAggObject::acquire(); }
    virtual void SAL_CALL release() { OWeakAggObject::release(); }

    virtual Any SAL_CALL queryAggregation( const Type& );


    // ::com::sun::star::lang::XTypeProvider

    // ::com::sun::star::lang::XServiceInfo
    ::rtl::OUString SAL_CALL getImplementationName() throw(  );

    sal_Bool SAL_CALL supportsService(const ::rtl::OUString& ServiceName) throw(  );
    Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw(  );
    static Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_Static(void) throw(  );
    static ::rtl::OUString SAL_CALL getImplementationName_Static() throw(  )
    {
        /** the soplayer uses this name in its source! maybe not after 5.2 */
        return ::rtl::OUString::createFromAscii( "com.sun.star.extensions.PluginModel" );
    }

    // OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
    virtual sal_Bool  SAL_CALL convertFastPropertyValue( Any & rConvertedValue,
                                                         Any & rOldValue,
                                                         sal_Int32 nHandle,
                                                         const Any& rValue );
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle,
                                                            const Any& rValue )
        throw( ::com::sun::star::lang::IllegalArgumentException );
    virtual void SAL_CALL getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const;
    virtual Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo();

    // ::com::sun::star::io::XPersistObject
    virtual ::rtl::OUString SAL_CALL getServiceName();
    virtual void SAL_CALL write(const Reference< ::com::sun::star::io::XObjectOutputStream > & OutStream);
    virtual void SAL_CALL read(const Reference< ::com::sun::star::io::XObjectInputStream > & InStream);

    // ::com::sun::star::lang::XComponent
    virtual void SAL_CALL addEventListener( const Reference< ::com::sun::star::lang::XEventListener > & l );
    virtual void SAL_CALL removeEventListener( const Reference< ::com::sun::star::lang::XEventListener > & l );
    virtual void SAL_CALL dispose();
};
Reference< XInterface >  SAL_CALL PluginModel_CreateInstance( const Reference< ::com::sun::star::lang::XMultiServiceFactory >  & ) throw( Exception );

#endif // __PLUGIN_MODEL_HXX
