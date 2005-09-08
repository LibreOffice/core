/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: backendlayerhelper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:36:20 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef CONFIGMGR_BACKENDHELPER_BACKENDLAYERHELPER_HXX_
#define CONFIGMGR_BACKENDHELPER_BACKENDLAYERHELPER_HXX_

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XLAYERCONTENTDESCRIBER_HPP_
#include <com/sun/star/configuration/backend/XLayerContentDescriber.hpp>
#endif


#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif // _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
#endif // _CPPUHELPER_COMPBASE2_HXX_

namespace configmgr { namespace backendhelper {

namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace lang = css::lang ;
namespace backend = css::configuration::backend ;
//------------------------------------------------------------------------------

typedef cppu::WeakComponentImplHelper2<backend::XLayerContentDescriber,
                                       lang::XServiceInfo> BackendBase ;

/**
  *  Implements the LayerContentDescriber service.
  *  Describes a set of configuration data to an XLayerHandler
  *  Object
  */
class BackendLayerHelper : public BackendBase {
public :
    /**
     Service constructor from a service factory.
     @param xContext   component context
    */
    BackendLayerHelper(const uno::Reference<uno::XComponentContext>& xContext) ;

    /** Destructor */
    ~BackendLayerHelper(void) ;


    // XServiceInfo
    virtual rtl::OUString SAL_CALL
        getImplementationName(  )
        throw (uno::RuntimeException) ;

    virtual sal_Bool SAL_CALL
        supportsService( const rtl::OUString& aServiceName )
        throw (uno::RuntimeException) ;

    virtual uno::Sequence<rtl::OUString> SAL_CALL
        getSupportedServiceNames(  )
        throw (uno::RuntimeException) ;


    //XLayerContentDescriber
    virtual void SAL_CALL
        describeLayer( const uno::Reference< backend::XLayerHandler >& xHandler,
                        const uno::Sequence< backend::PropertyInfo >& aPropertyInfos )
        throw (lang::NullPointerException,
               backend::MalformedDataException,
               uno::RuntimeException);

    /**
     Provides the implementation name.

     @return   implementation name
    */
    static rtl::OUString SAL_CALL getBackendLayerHelperName(void) ;
    /**
     Provides the supported services names

     @return   service names
    */
    static uno::Sequence<rtl::OUString> SAL_CALL getBackendLayerHelperServiceNames(void) ;
private:
    osl::Mutex mMutex;

} ;
//------------------------------------------------------------------------------
class OONode;
class OOProperty;

/**
   Base class for representing OO properties and nodes
*/
class IOONode
{
public:
    virtual OONode* getComposite(){return NULL;}
    virtual ~IOONode(){};
    virtual OOProperty* asOOProperty(){return NULL;}
    rtl::OUString getName(){return mName;}
    void  setName(const rtl::OUString& sName){mName = sName;}
protected:
    IOONode(const rtl::OUString& sName);
private:
    rtl::OUString mName;
};
//------------------------------------------------------------------------------
class OONode :public IOONode
{
public:

    OONode(const rtl::OUString& sName);
    OONode();
    ~OONode();

    IOONode* addChild(IOONode* aChild);
    OONode* getComposite(){return this;}
    const std::vector<IOONode*>& getChildren();
    IOONode* getChild(const rtl::OUString& aChildName);

private:
    typedef std::vector<IOONode*> ChildList;
    ChildList mChildList;
};
//------------------------------------------------------------------------------
class OOProperty :public IOONode
{
public:
    OOProperty(const rtl::OUString& sName,
               const rtl::OUString& sPropType,
               const uno::Any& aPropValue,
               sal_Bool bProtected);
    ~OOProperty(){};

    const rtl::OUString& getType(){return mPropType;}
    uno::Any getValue(){return mPropValue;}
    sal_Bool isProtected(){return mbProtected;}
    OOProperty* asOOProperty(){return this;}

private:
    rtl::OUString mPropType;
    uno::Any mPropValue;
    sal_Bool mbProtected;
};
//------------------------------------------------------------------------------


} } // configmgr.backendhelper

#endif // CONFIGMGR_BACKENDHELPER_BACKENDLAYERHELPER_HXX_


