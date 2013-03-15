/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef INFORMATIONDIALOG_HXX
#define INFORMATIONDIALOG_HXX
#include <vector>
#include "unodialog.hxx"
#include "configurationaccess.hxx"
#include "pppoptimizertoken.hxx"
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/awt/XItemListener.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/awt/XItemEventBroadcaster.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/awt/PushButtonType.hpp>
#include <com/sun/star/io/XStream.hpp>

// ---------------------
// - InformationDialog -
// ---------------------

class InformationDialog : public UnoDialog
                        , public ConfigurationAccess
{
public :

    InformationDialog( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxContext,
                       const com::sun::star::uno::Reference< com::sun::star::awt::XWindowPeer >& rxParent,
                       const rtl::OUString& rSaveAsURL,
                       sal_Bool& bOpenNewDocument,
                       const sal_Int64& nSourceSize,
                       const sal_Int64& nDestSize,
                       const sal_Int64& nApproxDest );
    ~InformationDialog();

    sal_Bool                execute();
private :

    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > mxContext;
    sal_Int64 mnSourceSize;
    sal_Int64 mnDestSize;
    sal_Int64 mnApproxSize;
    sal_Bool& mrbOpenNewDocument;
    const rtl::OUString& maSaveAsURL;
    com::sun::star::uno::Reference< com::sun::star::awt::XCheckBox > mxCheckBox;

    void InitDialog();
    com::sun::star::uno::Reference< com::sun::star::awt::XFixedText > InsertFixedText(
        const rtl::OUString& rControlName,
        const rtl::OUString& rLabel,
        sal_Int32 nXPos,
        sal_Int32 nYPos,
        sal_Int32 nWidth,
        sal_Int32 nHeight,
        sal_Bool bMultiLine,
        sal_Int16 nTabIndex );
    com::sun::star::uno::Reference< com::sun::star::awt::XControl > InsertImage(
        const rtl::OUString& rControlName,
        const rtl::OUString& rURL,
        sal_Int32 nPosX,
        sal_Int32 nPosY,
        sal_Int32 nWidth,
        sal_Int32 nHeight,
        sal_Bool bScale );
    com::sun::star::uno::Reference< com::sun::star::awt::XCheckBox > InsertCheckBox(
        const rtl::OUString& rControlName,
        const rtl::OUString& rLabel,
        const rtl::OUString& rHelpURL,
        sal_Int32 nXPos,
        sal_Int32 nYPos,
        sal_Int32 nWidth,
        sal_Int32 nHeight,
        sal_Int16 nTabIndex );
    com::sun::star::uno::Reference< com::sun::star::awt::XButton > InsertButton(
        const rtl::OUString& rControlName,
        sal_Int32 nXPos,
        sal_Int32 nYPos,
        sal_Int32 nWidth,
        sal_Int32 nHeight,
        sal_Int16 nTabIndex,
        sal_Int32 nResID );

};

#endif
