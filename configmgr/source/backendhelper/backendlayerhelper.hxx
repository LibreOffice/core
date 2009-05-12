/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: backendlayerhelper.hxx,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef CONFIGMGR_BACKENDHELPER_BACKENDLAYERHELPER_HXX_
#define CONFIGMGR_BACKENDHELPER_BACKENDLAYERHELPER_HXX_

#include <com/sun/star/configuration/backend/XLayerContentDescriber.hpp>


#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/compbase2.hxx>

namespace configmgr { namespace backendhelper {

namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace lang = css::lang ;
namespace backend = css::configuration::backend ;
//------------------------------------------------------------------------------

/**
  *  Implements the LayerContentDescriber service.
  *  Describes a set of configuration data to an XLayerHandler
  *  Object
  */
class BackendLayerHelper : public cppu::WeakComponentImplHelper2<backend::XLayerContentDescriber, lang::XServiceInfo> {
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
    std::vector<IOONode*> mChildList;
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


