/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: backendlayerhelper.cxx,v $
 * $Revision: 1.5 $
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


// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"


#include "backendlayerhelper.hxx"
#include <com/sun/star/configuration/backend/PropertyInfo.hpp>

namespace configmgr { namespace backendhelper {

//==============================================================================

//------------------------------------------------------------------------------
uno::Type toType(const ::rtl::OUString& _rType)
    {
        uno::Type aRet;

        if  (_rType.equalsIgnoreAsciiCase(::rtl::OUString::createFromAscii("boolean")))
            aRet = ::getBooleanCppuType();

        else if(_rType.equalsIgnoreAsciiCase(::rtl::OUString::createFromAscii("short")))
            aRet = ::getCppuType(static_cast<sal_Int16 const*>(0));

        else if(_rType.equalsIgnoreAsciiCase(::rtl::OUString::createFromAscii("int")))
            aRet = ::getCppuType(static_cast<sal_Int32 const*>(0));

        else if(_rType.equalsIgnoreAsciiCase(::rtl::OUString::createFromAscii("integer")))
            aRet = ::getCppuType(static_cast<sal_Int32 const*>(0));

        else if(_rType.equalsIgnoreAsciiCase(::rtl::OUString::createFromAscii("long")))
            aRet = ::getCppuType(static_cast<sal_Int64 const*>(0));

        else if(_rType.equalsIgnoreAsciiCase(::rtl::OUString::createFromAscii("double")))
            aRet = ::getCppuType(static_cast<double const*>(0));

        else if(_rType.equalsIgnoreAsciiCase(::rtl::OUString::createFromAscii("string")))
            aRet = ::getCppuType(static_cast<rtl::OUString const*>(0));

        else if(_rType.equalsIgnoreAsciiCase(::rtl::OUString::createFromAscii("binary")))
            aRet = ::getCppuType(static_cast<uno::Sequence<sal_Int8> const*>(0));

        else if(_rType.equalsIgnoreAsciiCase(::rtl::OUString::createFromAscii("any")))
            aRet = ::getCppuType(static_cast<uno::Any const*>(0));

        else if(_rType.equalsIgnoreAsciiCase(::rtl::OUString::createFromAscii("boolean-list")))
            aRet = ::getCppuType(static_cast<uno::Sequence<sal_Bool> const*>(0));

        else if(_rType.equalsIgnoreAsciiCase(::rtl::OUString::createFromAscii("short-list")))
            aRet = ::getCppuType(static_cast<uno::Sequence<sal_Int16> const*>(0));

        else if(_rType.equalsIgnoreAsciiCase(::rtl::OUString::createFromAscii("int-list")))
            aRet = ::getCppuType(static_cast<uno::Sequence<sal_Int32> const*>(0));

        else if(_rType.equalsIgnoreAsciiCase(::rtl::OUString::createFromAscii("integer-list")))
            aRet = ::getCppuType(static_cast<uno::Sequence<sal_Int32> const*>(0));

        else if(_rType.equalsIgnoreAsciiCase(::rtl::OUString::createFromAscii("long-list")))
            aRet = ::getCppuType(static_cast<uno::Sequence<sal_Int64> const*>(0));

        else if(_rType.equalsIgnoreAsciiCase(::rtl::OUString::createFromAscii("double-list")))
            aRet = ::getCppuType(static_cast<uno::Sequence<double> const*>(0));

        else if(_rType.equalsIgnoreAsciiCase(::rtl::OUString::createFromAscii("string-list")))
            aRet = ::getCppuType(static_cast<uno::Sequence<rtl::OUString> const*>(0));

        else if(_rType.equalsIgnoreAsciiCase(::rtl::OUString::createFromAscii("binary-list")))
            aRet = ::getCppuType(static_cast<uno::Sequence<uno::Sequence<sal_Int8> > const*>(0));

        else
        {
            ::rtl::OString aStr("Unknown type! ");
            aStr += rtl::OUStringToOString(_rType,RTL_TEXTENCODING_ASCII_US);
            OSL_ENSURE(0,aStr.getStr());
        }

        return aRet;
    }

//------------------------------------------------------------------------------

IOONode::IOONode(const rtl::OUString& sName):
    mName(sName)
{
}

//------------------------------------------------------------------------------
OOProperty::OOProperty(
    const rtl::OUString& sName,const rtl::OUString& sPropType,
    const uno::Any& aPropValue,sal_Bool bProtected)
    :IOONode(sName), mPropType(sPropType), mPropValue(aPropValue),
    mbProtected(bProtected)
{
}
//------------------------------------------------------------------------------
OONode::OONode(const rtl::OUString& sName)
    :IOONode(sName)
{
}

OONode::OONode()
    :IOONode(rtl::OUString())
{
}
IOONode* OONode::addChild(IOONode* aChild)
{
    mChildList.push_back(aChild);
    return aChild;
}
const std::vector<IOONode*>& OONode::getChildren()
{
    return mChildList;
}

IOONode* OONode::getChild(const rtl::OUString& aChildName)
{
    for (sal_uInt32 i=0; i< mChildList.size();++i)
    {
        if (mChildList[i]->getName() == aChildName)
            return mChildList[i];
    }
    return NULL;
}
OONode::~OONode()
{
    for (sal_uInt32 i=0; i< mChildList.size();++i)
    {
        delete mChildList[i];
    }
    mChildList.clear();
}
//------------------------------------------------------------------------------
 sal_Bool addChildrenToNodeTree(
     OONode * aNode,
     sal_Int32 nNextToken,
     const backend::PropertyInfo& aPropInfo,
     const uno::Reference<uno::XInterface>& xContext)
{
    do
    {
        rtl::OUString aName = aPropInfo.Name.getToken(0, '/',nNextToken);
        if (aName.getLength() == 0)
        {
            throw backend::MalformedDataException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                        "Malformed OpenOffice Key specified")),
                        xContext, uno::Any()) ;
        }
        //Check if Property -> nNextToken == -1
        if(nNextToken != -1)
        {
            //check if child already exists
            IOONode* aChildNode= aNode->getChild(aName);
            if (aChildNode == NULL)
            {
                aChildNode = new OONode(aName);
                if (aChildNode != 0)
                {
                    aNode->addChild( aChildNode);
                }
            }

            sal_Bool bFinished =addChildrenToNodeTree(
                aChildNode->getComposite(),
                nNextToken,
                aPropInfo,
                xContext);
            //Check that if you have finished parsing string therefore no
            //more children
            if (bFinished)
                break;
        }
        else
        {
            //Add Property
            IOONode* aProperty = new OOProperty(aName,
                                                aPropInfo.Type,
                                                aPropInfo.Value,
                                                aPropInfo.Protected);
            if (aProperty != 0)
            {
                aNode->addChild( aProperty);
            }
            //Return finished is true when you are finished parsing the string
            if( nNextToken == -1)
            {
                return sal_True;
            }
        }
    }
    while (nNextToken >= 0 ) ;
    return sal_True;
}
//------------------------------------------------------------------------------
void processChildren(
     std::vector<IOONode*> aChildList,
    const uno::Reference<backend::XLayerHandler>& xHandler)
{
    for(sal_uInt32 i=0; i <aChildList.size(); ++i)
    {
        OONode * aTestOONode = aChildList[i]->getComposite();
        if (aTestOONode)
        {
            xHandler->overrideNode(aTestOONode->getName(),0,false);
            processChildren(aTestOONode->getChildren(),xHandler);
            xHandler->endNode();
        }
        else
        {
            OOProperty* aProperty = aChildList[i]->asOOProperty();
            sal_Int16 aAttributes = aProperty->isProtected() ? 256:0;
            //Convert Type either simple or list
            uno::Type aType = toType( aProperty->getType());

            xHandler->overrideProperty(aProperty->getName(),
                                       aAttributes,
                                       aType,
                                       false);

            xHandler->setPropertyValue(aProperty->getValue());
            xHandler->endProperty();
        }

    }
}
//------------------------------------------------------------------------------
void buildNodeTree(
    const uno::Sequence< backend::PropertyInfo >& aPropertyInfos,
    const uno::Reference<uno::XInterface>& xContext,
    OONode& aNodeTree)
{
    sal_Int32 nNextToken =0;
    rtl::OUString aName = aPropertyInfos[0].Name.getToken(0, '/',nNextToken);
    if((nNextToken ==-1)||(aName.getLength()==0))
    {
         throw backend::MalformedDataException(
               rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                       "Malformed OpenOffice Key specified")),
                    xContext, uno::Any()) ;

    }
    aNodeTree.setName(aName);
    sal_Int32 size = aPropertyInfos.getLength();
    for (sal_Int32 i =0; i < size; ++i)
    {
        addChildrenToNodeTree(&aNodeTree, nNextToken,aPropertyInfos[i],xContext);
    }

}
//------------------------------------------------------------------------------
BackendLayerHelper::BackendLayerHelper(
    const uno::Reference<uno::XComponentContext>& /*xContext*/)
    :cppu::WeakComponentImplHelper2<backend::XLayerContentDescriber, lang::XServiceInfo>(mMutex)
{
}
//------------------------------------------------------------------------------

BackendLayerHelper::~BackendLayerHelper(void) {}

//------------------------------------------------------------------------------

rtl::OUString SAL_CALL BackendLayerHelper::
    getBackendLayerHelperName(void)
{
    static const rtl::OUString kImplementationName(
            RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.comp.configuration.backend.LayerDescriber")) ;

    return kImplementationName ;
}
//------------------------------------------------------------------------------
void SAL_CALL BackendLayerHelper::describeLayer(
    const uno::Reference< backend::XLayerHandler >& xHandler,
    const uno::Sequence< backend::PropertyInfo >& aPropertyInfos )
        throw (lang::NullPointerException,
               backend::MalformedDataException,
               uno::RuntimeException)



{
    OONode aNodeTree;
    buildNodeTree(aPropertyInfos, *this, aNodeTree);

    //Descirbe the Layer to the XHandler Object
    xHandler->startLayer();
    xHandler->overrideNode(aNodeTree.getName(),0,false);
    std::vector<IOONode*> aChildList = aNodeTree.getChildren();
    processChildren(aChildList,xHandler);
    xHandler->endNode();
    xHandler->endLayer();


}
//------------------------------------------------------------------------------

rtl::OUString SAL_CALL BackendLayerHelper::getImplementationName(void)
    throw (uno::RuntimeException)
{
    return getBackendLayerHelperName() ;
}
//------------------------------------------------------------------------------

uno::Sequence<rtl::OUString> SAL_CALL BackendLayerHelper::
    getBackendLayerHelperServiceNames(void)
{
    uno::Sequence<rtl::OUString> aServices(1) ;
    aServices[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.backend.LayerDescriber")) ;
    return aServices ;
}
//------------------------------------------------------------------------------

sal_Bool SAL_CALL BackendLayerHelper::supportsService(
    const rtl::OUString& aServiceName)
    throw (uno::RuntimeException)
{
    uno::Sequence< rtl::OUString > const svc = getBackendLayerHelperServiceNames();

    for(sal_Int32 i = 0; i < svc.getLength(); ++i )
        if(svc[i] == aServiceName)
            return true;
    return false;
}
//------------------------------------------------------------------------------

uno::Sequence<rtl::OUString>
SAL_CALL BackendLayerHelper::getSupportedServiceNames(void)
    throw (uno::RuntimeException)
{
    return getBackendLayerHelperServiceNames() ;
}
//------------------------------------------------------------------------------

} // backendhelper
} // configmgr
