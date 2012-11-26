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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basctl.hxx"

#include "dlgedfac.hxx"
#include "dlgedobj.hxx"
#include <dlgeddef.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/ScrollBarOrientation.hpp>
#include <svx/sdrobjectfactory.hxx>

using namespace ::com::sun::star;

//----------------------------------------------------------------------------

DlgEdFactory::DlgEdFactory()
{
    SdrObjFactory::InsertMakeObjectHdl( LINK(this, DlgEdFactory, MakeObject) );
}

//----------------------------------------------------------------------------

DlgEdFactory::~DlgEdFactory()
{
    SdrObjFactory::RemoveMakeObjectHdl( LINK(this, DlgEdFactory, MakeObject) );
}

//----------------------------------------------------------------------------

IMPL_LINK( DlgEdFactory, MakeObject, SdrObjFactory *, pObjFactory )
{
    static sal_Bool bNeedsInit = sal_True;
    static uno::Reference< lang::XMultiServiceFactory > xDialogSFact;

    if( bNeedsInit )
    {
        uno::Reference< lang::XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
        uno::Reference< container::XNameContainer > xC( xMSF->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlDialogModel" ) ) ), uno::UNO_QUERY );
        if( xC.is() )
        {
            uno::Reference< lang::XMultiServiceFactory > xModFact( xC, uno::UNO_QUERY );
            xDialogSFact = xModFact;
        }
        bNeedsInit = sal_False;
    }

    if( (DlgInventor == pObjFactory->getSdrObjectCreationInfo().getInvent()) &&
        (pObjFactory->getSdrObjectCreationInfo().getIdent() >= OBJ_DLG_PUSHBUTTON) &&
        (pObjFactory->getSdrObjectCreationInfo().getIdent() <= OBJ_DLG_TREECONTROL)    )
    {
        switch(pObjFactory->getSdrObjectCreationInfo().getIdent())
        {
            case OBJ_DLG_PUSHBUTTON:
            {
                 pObjFactory->setNewSdrObject(
                     new DlgEdObj(
                        pObjFactory->getTargetModel(),
                        ::rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlButtonModel") , xDialogSFact ));
                  break;
            }
            case OBJ_DLG_RADIOBUTTON:
            {
                 pObjFactory->setNewSdrObject(
                     new DlgEdObj(
                        pObjFactory->getTargetModel(),
                        ::rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlRadioButtonModel") , xDialogSFact ));
                 break;
            }
            case OBJ_DLG_CHECKBOX:
            {
                 pObjFactory->setNewSdrObject(
                     new DlgEdObj(
                         pObjFactory->getTargetModel(),
                         ::rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlCheckBoxModel") , xDialogSFact ));
                 break;
            }
            case OBJ_DLG_LISTBOX:
            {
                 pObjFactory->setNewSdrObject(
                     new DlgEdObj(
                         pObjFactory->getTargetModel(),
                         ::rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlListBoxModel") , xDialogSFact ));
                 break;
            }
            case OBJ_DLG_COMBOBOX:
            {
                DlgEdObj* pNew = new DlgEdObj(
                     pObjFactory->getTargetModel(),
                    ::rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlComboBoxModel") , xDialogSFact );
                pObjFactory->setNewSdrObject(pNew);
                try
                {
                    uno::Reference< beans::XPropertySet >  xPSet(pNew->GetUnoControlModel(), uno::UNO_QUERY);
                    if (xPSet.is())
                    {
                        sal_Bool bB = sal_True;
                        xPSet->setPropertyValue( DLGED_PROP_DROPDOWN, uno::Any(&bB,::getBooleanCppuType()));
                    }
                }
                catch(...)
                {
                }
                break;
            }
            case OBJ_DLG_GROUPBOX:
            {
                 pObjFactory->setNewSdrObject(
                     new DlgEdObj(
                         pObjFactory->getTargetModel(),
                         ::rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlGroupBoxModel") , xDialogSFact ));
                 break;
            }
            case OBJ_DLG_EDIT:
            {
                 pObjFactory->setNewSdrObject(
                     new DlgEdObj(
                         pObjFactory->getTargetModel(),
                         ::rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlEditModel") , xDialogSFact ));
                 break;
            }
            case OBJ_DLG_FIXEDTEXT:
            {
                 pObjFactory->setNewSdrObject(
                     new DlgEdObj(
                         pObjFactory->getTargetModel(),
                         ::rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlFixedTextModel") , xDialogSFact ));
                 break;
            }
            case OBJ_DLG_IMAGECONTROL:
            {
                 pObjFactory->setNewSdrObject(
                     new DlgEdObj(
                         pObjFactory->getTargetModel(),
                         ::rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlImageControlModel") , xDialogSFact ));
                 break;
            }
            case OBJ_DLG_PROGRESSBAR:
            {
                 pObjFactory->setNewSdrObject(
                     new DlgEdObj(
                         pObjFactory->getTargetModel(),
                         ::rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlProgressBarModel") , xDialogSFact ));
                 break;
            }
            case OBJ_DLG_HSCROLLBAR:
            {
                 pObjFactory->setNewSdrObject(
                     new DlgEdObj(
                         pObjFactory->getTargetModel(),
                         ::rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlScrollBarModel") , xDialogSFact ));
                 break;
            }
            case OBJ_DLG_VSCROLLBAR:
            {
                 DlgEdObj* pNew = new DlgEdObj(
                     pObjFactory->getTargetModel(),
                     ::rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlScrollBarModel") , xDialogSFact );
                 pObjFactory->setNewSdrObject(pNew);
                 // set vertical orientation
                 try
                 {
                    uno::Reference< beans::XPropertySet >  xPSet(pNew->GetUnoControlModel(), uno::UNO_QUERY);
                    if (xPSet.is())
                    {
                        uno::Any aValue;
                        aValue <<= (sal_Int32) ::com::sun::star::awt::ScrollBarOrientation::VERTICAL;
                        xPSet->setPropertyValue( DLGED_PROP_ORIENTATION, aValue );
                    }
                 }
                 catch(...)
                 {
                 }
                 break;
            }
            case OBJ_DLG_HFIXEDLINE:
            {
                 pObjFactory->setNewSdrObject(
                     new DlgEdObj(
                         pObjFactory->getTargetModel(),
                         ::rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlFixedLineModel") , xDialogSFact ));
                 break;
            }
            case OBJ_DLG_VFIXEDLINE:
            {
                 DlgEdObj* pNew = new DlgEdObj(
                     pObjFactory->getTargetModel(),
                     ::rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlFixedLineModel") , xDialogSFact );
                 pObjFactory->setNewSdrObject(pNew);
                 // set vertical orientation
                 try
                 {
                    uno::Reference< beans::XPropertySet >  xPSet(pNew->GetUnoControlModel(), uno::UNO_QUERY);
                    if (xPSet.is())
                    {
                        uno::Any aValue;
                        aValue <<= (sal_Int32) 1;
                        xPSet->setPropertyValue( DLGED_PROP_ORIENTATION, aValue );
                    }
                 }
                 catch(...)
                 {
                 }
                 break;
            }
            case OBJ_DLG_DATEFIELD:
            {
                 pObjFactory->setNewSdrObject(
                     new DlgEdObj(
                         pObjFactory->getTargetModel(),
                         ::rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlDateFieldModel") , xDialogSFact ));
                 break;
            }
            case OBJ_DLG_TIMEFIELD:
            {
                 pObjFactory->setNewSdrObject(
                     new DlgEdObj(
                         pObjFactory->getTargetModel(),
                         ::rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlTimeFieldModel") , xDialogSFact ));
                 break;
            }
            case OBJ_DLG_NUMERICFIELD:
            {
                 pObjFactory->setNewSdrObject(
                     new DlgEdObj(
                         pObjFactory->getTargetModel(),
                         ::rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlNumericFieldModel") , xDialogSFact ));
                 break;
            }
            case OBJ_DLG_CURRENCYFIELD:
            {
                 pObjFactory->setNewSdrObject(
                     new DlgEdObj(
                         pObjFactory->getTargetModel(),
                         ::rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlCurrencyFieldModel") , xDialogSFact ));
                 break;
            }
            case OBJ_DLG_FORMATTEDFIELD:
            {
                 pObjFactory->setNewSdrObject(
                     new DlgEdObj(
                         pObjFactory->getTargetModel(),
                         ::rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlFormattedFieldModel") , xDialogSFact ));
                 break;
            }
            case OBJ_DLG_PATTERNFIELD:
            {
                 pObjFactory->setNewSdrObject(
                     new DlgEdObj(
                         pObjFactory->getTargetModel(),
                         ::rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlPatternFieldModel") , xDialogSFact ));
                 break;
            }
            case OBJ_DLG_FILECONTROL:
            {
                 pObjFactory->setNewSdrObject(
                     new DlgEdObj(
                         pObjFactory->getTargetModel(),
                         ::rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlFileControlModel") , xDialogSFact ));
                 break;
            }
            case OBJ_DLG_TREECONTROL:
            {
                 DlgEdObj* pNew = new DlgEdObj(
                     pObjFactory->getTargetModel(),
                     ::rtl::OUString::createFromAscii("com.sun.star.awt.tree.TreeControlModel") , xDialogSFact );
                 pObjFactory->setNewSdrObject(pNew);
                 /*
                 try
                 {
                    uno::Reference< beans::XPropertySet >  xPSet(pNew->GetUnoControlModel(), uno::UNO_QUERY);
                    if (xPSet.is())
                    {
                        // first create a data model for our tree control
                        Reference< XComponentContext > xComponentContext;

                        Reference< XPropertySet > xPropSet( xInterface, UNO_QUERY );
                        xPropSet->getPropertyValue( OUString::createFromAscii("DefaultContext") ) >>= xComponentContext;

                        // gets the service manager from the office
                        Reference< XMultiComponentFactory > xMultiComponentFactoryServer( xComponentContext->getServiceManager() );


                        // gets the TreeDataModel
                        Reference< XMutableTreeDataModel > xTreeDataModel;

                        xTreeDataModel = Reference< XMutableTreeDataModel >(
                         xMultiComponentFactoryServer->createInstanceWithContext(
                        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.tree.MutableTreeDataModel" ) ), xComponentContext ), UNO_QUERY_THROW );

                        // now fill it with some sample data
                        const OUString sRoot( RTL_CONSTASCII_USTRINGPARAM( "Root" ) );

                        Reference< XMutableTreeNode > xNode( mxTreeDataModel->createNode( sRoot, false ), UNO_QUERY_THROW );
                        xNode->setDataValue( sRoot );
                        xNode->setExpandedGraphicURL( OUString( RTL_CONSTASCII_USTRINGPARAM( "private:graphicrepository/sd/res/triangle_down.png" ) ) );
                        xNode->setCollapsedGraphicURL( OUString( RTL_CONSTASCII_USTRINGPARAM( "private:graphicrepository/sd/res/triangle_right.png" ) ) );

                        const OUString sNode_1( RTL_CONSTASCII_USTRINGPARAM( "Node_1" ) );

                        Reference< XMutableTreeNode > xChildNode_1( mxTreeDataModel->createNode( sNode_1, true ), UNO_QUERY_THROW );
                        xChildNode_1->setDataValue( sNode_1 );
                        xChildNode_1->setExpandedGraphicURL( OUString( RTL_CONSTASCII_USTRINGPARAM( "private:graphicrepository/sd/res/triangle_down.png" ) ) );
                        xChildNode_1->setCollapsedGraphicURL( OUString( RTL_CONSTASCII_USTRINGPARAM( "private:graphicrepository/sd/res/triangle_right.png" ) ) );

                        xNode->appendChild( xChildNode_1 );

                        const OUString sNode_1_1( RTL_CONSTASCII_USTRINGPARAM( "Node_1_1" ) );

                        Reference< XMutableTreeNode > xChildNode_1_1( mxTreeDataModel->createNode( sNode_1_1, false ), UNO_QUERY_THROW );
                        xChildNode_1_1->setDataValue( sNode_1_1 );
                        xChildNode_1_1->setExpandedGraphicURL( OUString( RTL_CONSTASCII_USTRINGPARAM( "private:graphicrepository/sd/res/triangle_down.png" ) ) );
                        xChildNode_1_1->setCollapsedGraphicURL( OUString( RTL_CONSTASCII_USTRINGPARAM( "private:graphicrepository/sd/res/triangle_right.png" ) ) );

                        xChildNode_1->appendChild( xChildNode_1_1 );

                        const OUString sNode_1_1( RTL_CONSTASCII_USTRINGPARAM( "Node_2" ) );

                        Reference< XMutableTreeNode > xChildNode_2( mxTreeDataModel->createNode( sNode_2, false ), UNO_QUERY_THROW );
                        xChildNode_2->setDataValue( sNode_2 );
                        xChildNode_2->setNodeGraphicURL( OUString( RTL_CONSTASCII_USTRINGPARAM("private:graphicrepository/sw/imglst/nc20010.png") ) );
                        xNode->appendChild( xChildNode_2 );

                        xTreeDataModel->setRoot( xNode );


                        const OUString sDataModel( RTL_CONSTASCII_USTRINGPARAM( "DataModel" ) );

                        xPSet->setPropertyValue( sDataModel, xTreeDataModel );
                    }
                 }
                 catch(...)
                 {
                 }*/
                 break;
        }
    }
    }

    return 0;
}

//----------------------------------------------------------------------------

