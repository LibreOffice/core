/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <sal/macros.h>
#include "OfficeControlAccess.hxx"
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ControlActions.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <tools/urlobj.hxx>
#include <tools/debug.hxx>

#include <algorithm>


namespace svt
{


    // helper -------------------------------------------------------------

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::ui::dialogs;

    using namespace ExtendedFilePickerElementIds;
    using namespace CommonFilePickerElementIds;
    using namespace InternalFilePickerElementIds;


    namespace
    {

        struct ControlDescription
        {
            const char*     pControlName;
            sal_Int16       nControlId;
            PropFlags       nPropertyFlags;
        };


        typedef const ControlDescription* ControlDescIterator;


        #define PROPERTY_FLAGS_COMMON       ( PropFlags::Enabled | PropFlags::Visible | PropFlags::HelpUrl )
        #define PROPERTY_FLAGS_LISTBOX      ( PropFlags::ListItems | PropFlags::SelectedItem | PropFlags::SelectedItemIndex )
        #define PROPERTY_FLAGS_CHECKBOX     ( PropFlags::Checked | PropFlags::Text )

        // Note: this array MUST be sorted by name!
        static const ControlDescription aDescriptions[] =  {
            { "AutoExtensionBox",       CHECKBOX_AUTOEXTENSION,         PROPERTY_FLAGS_COMMON | PROPERTY_FLAGS_CHECKBOX     },
            { "CancelButton",           PUSHBUTTON_CANCEL,              PROPERTY_FLAGS_COMMON | PropFlags::Text          },
            { "CurrentFolderText",      FIXEDTEXT_CURRENTFOLDER,        PROPERTY_FLAGS_COMMON | PropFlags::Text          },
            { "DefaultLocationButton",  TOOLBOXBUTOON_DEFAULT_LOCATION, PROPERTY_FLAGS_COMMON                               },
            { "FileURLEdit",            EDIT_FILEURL,                   PROPERTY_FLAGS_COMMON | PropFlags::Text          },
            { "FileURLEditLabel",       EDIT_FILEURL_LABEL,             PROPERTY_FLAGS_COMMON | PropFlags::Text          },
            { "FileView",               CONTROL_FILEVIEW,               PROPERTY_FLAGS_COMMON                               },
            { "FilterList",             LISTBOX_FILTER,                 PROPERTY_FLAGS_COMMON                               },
            { "FilterListLabel",        LISTBOX_FILTER_LABEL,           PROPERTY_FLAGS_COMMON | PropFlags::Text          },
            { "FilterOptionsBox",       CHECKBOX_FILTEROPTIONS,         PROPERTY_FLAGS_COMMON | PROPERTY_FLAGS_CHECKBOX     },
            { "GpgPassword",            CHECKBOX_GPGENCRYPTION,         PROPERTY_FLAGS_COMMON | PROPERTY_FLAGS_CHECKBOX     },
            { "HelpButton",             PUSHBUTTON_HELP,                PROPERTY_FLAGS_COMMON | PropFlags::Text          },
            { "ImageAnchorList",        LISTBOX_IMAGE_ANCHOR,           PROPERTY_FLAGS_COMMON | PROPERTY_FLAGS_LISTBOX      },
            { "ImageAnchorListLabel",   LISTBOX_IMAGE_ANCHOR_LABEL,     PROPERTY_FLAGS_COMMON | PropFlags::Text             },
            { "ImageTemplateList",      LISTBOX_IMAGE_TEMPLATE,         PROPERTY_FLAGS_COMMON | PROPERTY_FLAGS_LISTBOX      },
            { "ImageTemplateListLabel", LISTBOX_IMAGE_TEMPLATE_LABEL,   PROPERTY_FLAGS_COMMON | PropFlags::Text          },
            { "LevelUpButton",          TOOLBOXBUTOON_LEVEL_UP,         PROPERTY_FLAGS_COMMON                               },
            { "LinkBox",                CHECKBOX_LINK,                  PROPERTY_FLAGS_COMMON | PROPERTY_FLAGS_CHECKBOX     },
            { "NewFolderButton",        TOOLBOXBUTOON_NEW_FOLDER,       PROPERTY_FLAGS_COMMON                               },
            { "OkButton",               PUSHBUTTON_OK ,                 PROPERTY_FLAGS_COMMON | PropFlags::Text          },
            { "PasswordBox",            CHECKBOX_PASSWORD,              PROPERTY_FLAGS_COMMON | PROPERTY_FLAGS_CHECKBOX     },
            { "PlayButton",             PUSHBUTTON_PLAY,                PROPERTY_FLAGS_COMMON | PropFlags::Text          },
            { "PreviewBox",             CHECKBOX_PREVIEW,               PROPERTY_FLAGS_COMMON | PROPERTY_FLAGS_CHECKBOX     },
            { "ReadOnlyBox",            CHECKBOX_READONLY,              PROPERTY_FLAGS_COMMON | PROPERTY_FLAGS_CHECKBOX     },
            { "SelectionBox",           CHECKBOX_SELECTION,             PROPERTY_FLAGS_COMMON | PROPERTY_FLAGS_CHECKBOX     },
            { "TemplateList",           LISTBOX_TEMPLATE,               PROPERTY_FLAGS_COMMON | PROPERTY_FLAGS_LISTBOX      },
            { "TemplateListLabel",      LISTBOX_TEMPLATE_LABEL,         PROPERTY_FLAGS_COMMON | PropFlags::Text          },
            { "VersionList",            LISTBOX_VERSION,                PROPERTY_FLAGS_COMMON | PROPERTY_FLAGS_LISTBOX      },
            { "VersionListLabel",       LISTBOX_VERSION_LABEL,          PROPERTY_FLAGS_COMMON | PropFlags::Text          }
        };

        static const sal_Int32 s_nControlCount = SAL_N_ELEMENTS( aDescriptions );

        static ControlDescIterator s_pControls = aDescriptions;
        static ControlDescIterator s_pControlsEnd = aDescriptions + s_nControlCount;

        struct ControlDescriptionLookup
        {
            bool operator()( const ControlDescription& rDesc1, const ControlDescription& rDesc2 )
            {
                return strcmp(rDesc1.pControlName, rDesc2.pControlName) < 0;
            }
        };

        struct ControlProperty
        {
            const char*     pPropertyName;
            PropFlags       nPropertyId;
        };

        typedef const ControlProperty* ControlPropertyIterator;

        static const ControlProperty aProperties[] =  {
            { "Text",               PropFlags::Text              },
            { "Enabled",            PropFlags::Enabled          },
            { "Visible",            PropFlags::Visible           },
            { "HelpURL",            PropFlags::HelpUrl           },
            { "ListItems",          PropFlags::ListItems         },
            { "SelectedItem",       PropFlags::SelectedItem      },
            { "SelectedItemIndex",  PropFlags::SelectedItemIndex },
            { "Checked",            PropFlags::Checked           }
        };

        static const int s_nPropertyCount = SAL_N_ELEMENTS( aProperties );

        static ControlPropertyIterator s_pProperties = aProperties;
        static ControlPropertyIterator s_pPropertiesEnd = aProperties + s_nPropertyCount;


        struct ControlPropertyLookup
        {
            OUString m_sLookup;
            explicit ControlPropertyLookup(const OUString& rLookup)
                : m_sLookup(rLookup)
            {
            }

            bool operator()(const ControlProperty& rProp)
            {
                return m_sLookup.equalsAscii(rProp.pPropertyName);
            }
        };


        void lcl_throwIllegalArgumentException( )
        {
            throw IllegalArgumentException();
            // TODO: error message in the exception
        }
    }

    OControlAccess::OControlAccess(IFilePickerController* pController, SvtFileView* pFileView)
        : m_pFilePickerController(pController)
        , m_pFileView(pFileView)
    {
        DBG_ASSERT( m_pFilePickerController, "OControlAccess::OControlAccess: invalid control locator!" );
    }

    bool OControlAccess::IsFileViewWidget(weld::Widget const * pControl) const
    {
        if (!pControl)
            return false;
        if (!m_pFileView)
            return false;
        return pControl == m_pFileView->identifier();
    }

    void OControlAccess::setHelpURL(weld::Widget* pControl, const OUString& sHelpURL)
    {
        OUString sHelpID( sHelpURL );
        INetURLObject aHID( sHelpURL );
        if (aHID.GetProtocol() == INetProtocol::Hid)
            sHelpID = aHID.GetURLPath();

        // URLs should always be UTF8 encoded and escaped
        OString sID( OUStringToOString( sHelpID, RTL_TEXTENCODING_UTF8 ) );
        if (IsFileViewWidget(pControl))
        {
            // the file view "overrides" the SetHelpId
            m_pFileView->set_help_id(sID);
        }
        else
            pControl->set_help_id(sID);
    }

    OUString OControlAccess::getHelpURL(weld::Widget const * pControl) const
    {
        OString aHelpId = pControl->get_help_id();
        if (IsFileViewWidget(pControl))
        {
            // the file view "overrides" the SetHelpId
            aHelpId = m_pFileView->get_help_id();
        }

        OUString sHelpURL;
        OUString aTmp( OStringToOUString( aHelpId, RTL_TEXTENCODING_UTF8 ) );
        INetURLObject aHID( aTmp );
        if ( aHID.GetProtocol() == INetProtocol::NotValid )
            sHelpURL = INET_HID_SCHEME;
        sHelpURL += aTmp;
        return sHelpURL;
    }

    Any OControlAccess::getControlProperty( const OUString& rControlName, const OUString& rControlProperty )
    {
        // look up the control
        sal_Int16 nControlId = -1;
        PropFlags nPropertyMask = PropFlags::NONE;
        weld::Widget* pControl = implGetControl( rControlName, &nControlId, &nPropertyMask );
            // will throw an IllegalArgumentException if the name is not valid

        // look up the property
        ControlPropertyIterator aPropDesc = ::std::find_if( s_pProperties, s_pPropertiesEnd, ControlPropertyLookup( rControlProperty ) );
        if ( aPropDesc == s_pPropertiesEnd )
            // it's a completely unknown property
            lcl_throwIllegalArgumentException();

        if ( !( nPropertyMask & aPropDesc->nPropertyId ) )
            // it's a property which is known, but not allowed for this control
            lcl_throwIllegalArgumentException();

        return implGetControlProperty( pControl, aPropDesc->nPropertyId );
    }

    weld::Widget* OControlAccess::implGetControl( const OUString& rControlName, sal_Int16* _pId, PropFlags* _pPropertyMask ) const
    {
        weld::Widget* pControl = nullptr;
        ControlDescription tmpDesc;
        OString aControlName = OUStringToOString( rControlName, RTL_TEXTENCODING_UTF8 );
        tmpDesc.pControlName = aControlName.getStr();

        // translate the name into an id
        auto aFoundRange = ::std::equal_range( s_pControls, s_pControlsEnd, tmpDesc, ControlDescriptionLookup() );
        if ( aFoundRange.first != aFoundRange.second )
        {
            // get the VCL control determined by this id
            pControl = m_pFilePickerController->getControl( aFoundRange.first->nControlId );
        }

        // if not found 'til here, the name is invalid, or we do not have the control in the current mode
        if ( !pControl )
            lcl_throwIllegalArgumentException();

        // out parameters and outta here
        if ( _pId )
            *_pId = aFoundRange.first->nControlId;
        if ( _pPropertyMask )
            *_pPropertyMask = aFoundRange.first->nPropertyFlags;

        return pControl;
    }

    void OControlAccess::setControlProperty( const OUString& rControlName, const OUString& rControlProperty, const css::uno::Any& rValue )
    {
        // look up the control
        sal_Int16 nControlId = -1;
        weld::Widget* pControl = implGetControl( rControlName, &nControlId );
            // will throw an IllegalArgumentException if the name is not valid

        // look up the property
        ControlPropertyIterator aPropDesc = ::std::find_if( s_pProperties, s_pPropertiesEnd, ControlPropertyLookup( rControlProperty ) );
        if ( aPropDesc == s_pPropertiesEnd )
            lcl_throwIllegalArgumentException();

        // set the property
        implSetControlProperty( nControlId, pControl, aPropDesc->nPropertyId, rValue, false );
    }

    Sequence< OUString > OControlAccess::getSupportedControls(  ) const
    {
        Sequence< OUString > aControls( s_nControlCount );
        OUString* pControls = aControls.getArray();

        // collect the names of all _actually_existent_ controls
        for ( ControlDescIterator aControl = s_pControls; aControl != s_pControlsEnd; ++aControl )
        {
            if ( m_pFilePickerController->getControl( aControl->nControlId ) )
                *pControls++ = OUString::createFromAscii( aControl->pControlName );
        }

        aControls.realloc( pControls - aControls.getArray() );
        return aControls;
    }

    Sequence< OUString > OControlAccess::getSupportedControlProperties( const OUString& rControlName )
    {
        sal_Int16 nControlId = -1;
        PropFlags nPropertyMask = PropFlags::NONE;
        implGetControl( rControlName, &nControlId, &nPropertyMask );
            // will throw an IllegalArgumentException if the name is not valid

        // fill in the property names
        Sequence< OUString > aProps( s_nPropertyCount );
        OUString* pProperty = aProps.getArray();

        for ( ControlPropertyIterator aProp = s_pProperties; aProp != s_pPropertiesEnd; ++aProp )
            if ( nPropertyMask & aProp->nPropertyId )
                *pProperty++ = OUString::createFromAscii( aProp->pPropertyName );

        aProps.realloc( pProperty - aProps.getArray() );
        return aProps;
    }

    bool OControlAccess::isControlSupported( const OUString& rControlName )
    {
        ControlDescription tmpDesc;
        OString aControlName = OUStringToOString(rControlName, RTL_TEXTENCODING_UTF8);
        tmpDesc.pControlName = aControlName.getStr();
        return ::std::binary_search( s_pControls, s_pControlsEnd, tmpDesc, ControlDescriptionLookup() );
    }

    bool OControlAccess::isControlPropertySupported( const OUString& rControlName, const OUString& rControlProperty )
    {
        // look up the control
        sal_Int16 nControlId = -1;
        PropFlags nPropertyMask = PropFlags::NONE;
        implGetControl( rControlName, &nControlId, &nPropertyMask );
            // will throw an IllegalArgumentException if the name is not valid

        // look up the property
        ControlPropertyIterator aPropDesc = ::std::find_if( s_pProperties, s_pPropertiesEnd, ControlPropertyLookup( rControlProperty ) );
        if ( aPropDesc == s_pPropertiesEnd )
            // it's a property which is completely unknown
            return false;

        return bool( aPropDesc->nPropertyId & nPropertyMask );
    }

    void OControlAccess::setValue( sal_Int16 nControlId, sal_Int16 nControlAction, const Any& rValue )
    {
        weld::Widget* pControl = m_pFilePickerController->getControl( nControlId );
        DBG_ASSERT( pControl, "OControlAccess::SetValue: don't have this control in the current mode!" );
        if ( !pControl )
            return;

        PropFlags nPropertyId = PropFlags::Unknown;
        if ( ControlActions::SET_HELP_URL == nControlAction )
        {
            nPropertyId = PropFlags::HelpUrl;
        }
        else
        {
            switch ( nControlId )
            {
                case CHECKBOX_AUTOEXTENSION:
                case CHECKBOX_PASSWORD:
                case CHECKBOX_FILTEROPTIONS:
                case CHECKBOX_READONLY:
                case CHECKBOX_LINK:
                case CHECKBOX_PREVIEW:
                case CHECKBOX_SELECTION:
                    nPropertyId = PropFlags::Checked;
                    break;

                case LISTBOX_FILTER:
                    SAL_WARN( "fpicker.office", "Use the XFilterManager to access the filter listbox" );
                    break;

                case LISTBOX_VERSION:
                case LISTBOX_TEMPLATE:
                case LISTBOX_IMAGE_TEMPLATE:
                case LISTBOX_IMAGE_ANCHOR:
                    if ( ControlActions::SET_SELECT_ITEM == nControlAction )
                    {
                        nPropertyId = PropFlags::SelectedItemIndex;
                    }
                    else
                    {
                        weld::ComboBox* pComboBox = dynamic_cast<weld::ComboBox*>(pControl);
                        assert(pComboBox && "OControlAccess::SetValue: implGetControl returned nonsense!");
                        implDoListboxAction(pComboBox, nControlAction, rValue);
                    }
                    break;
            }
        }

        if ( PropFlags::Unknown != nPropertyId )
            implSetControlProperty( nControlId, pControl, nPropertyId, rValue );
    }

    Any OControlAccess::getValue( sal_Int16 nControlId, sal_Int16 nControlAction ) const
    {
        Any aRet;

        weld::Widget* pControl = m_pFilePickerController->getControl( nControlId );
        DBG_ASSERT( pControl, "OControlAccess::GetValue: don't have this control in the current mode!" );
        if ( pControl )
        {
            PropFlags nPropertyId = PropFlags::Unknown;
            if ( ControlActions::SET_HELP_URL == nControlAction )
            {
                nPropertyId = PropFlags::HelpUrl;
            }
            else
            {
                switch ( nControlId )
                {
                    case CHECKBOX_AUTOEXTENSION:
                    case CHECKBOX_PASSWORD:
                    case CHECKBOX_GPGENCRYPTION:
                    case CHECKBOX_FILTEROPTIONS:
                    case CHECKBOX_READONLY:
                    case CHECKBOX_LINK:
                    case CHECKBOX_PREVIEW:
                    case CHECKBOX_SELECTION:
                        nPropertyId = PropFlags::Checked;
                        break;

                    case LISTBOX_FILTER:
                        if ( ControlActions::GET_SELECTED_ITEM == nControlAction )
                        {
                            aRet <<= m_pFilePickerController->getCurFilter();
                        }
                        else
                        {
                            SAL_WARN( "fpicker.office", "Use the XFilterManager to access the filter listbox" );
                        }
                        break;

                    case LISTBOX_VERSION:
                    case LISTBOX_TEMPLATE:
                    case LISTBOX_IMAGE_TEMPLATE:
                    case LISTBOX_IMAGE_ANCHOR:
                        switch ( nControlAction )
                        {
                            case ControlActions::GET_SELECTED_ITEM:
                                nPropertyId = PropFlags::SelectedItem;
                                break;
                            case ControlActions::GET_SELECTED_ITEM_INDEX:
                                nPropertyId = PropFlags::SelectedItemIndex;
                                break;
                            case ControlActions::GET_ITEMS:
                                nPropertyId = PropFlags::ListItems;
                                break;
                            default:
                                SAL_WARN( "fpicker.office", "OControlAccess::GetValue: invalid control action for the listbox!" );
                                break;
                        }
                        break;
                }
            }

            if ( PropFlags::Unknown != nPropertyId )
                aRet = implGetControlProperty( pControl, nPropertyId );
        }

        return aRet;
    }

    void OControlAccess::setLabel( sal_Int16 nId, const OUString &rLabel )
    {
        weld::Widget* pControl = m_pFilePickerController->getControl(nId, true);
        if (weld::Label* pLabel = dynamic_cast<weld::Label*>(pControl))
        {
            pLabel->set_label(rLabel);
            return;
        }
        if (weld::Button* pButton = dynamic_cast<weld::Button*>(pControl))
        {
            pButton->set_label(rLabel);
            return;
        }
        assert(false && "OControlAccess::GetValue: don't have this control in the current mode!");
    }

    OUString OControlAccess::getLabel( sal_Int16 nId ) const
    {
        weld::Widget* pControl = m_pFilePickerController->getControl(nId, true);
        if (weld::Label* pLabel = dynamic_cast<weld::Label*>(pControl))
            return pLabel->get_label();
        if (weld::Button* pButton = dynamic_cast<weld::Button*>(pControl))
            return pButton->get_label();
        assert(false && "OControlAccess::GetValue: don't have this control in the current mode!");
        return OUString();
    }

    void OControlAccess::enableControl(sal_Int16 nId, bool bEnable)
    {
        m_pFilePickerController->enableControl(nId, bEnable);
    }

    void OControlAccess::implDoListboxAction(weld::ComboBox* pListbox, sal_Int16 nControlAction, const Any& rValue)
    {
        switch ( nControlAction )
        {
            case ControlActions::ADD_ITEM:
            {
                OUString aEntry;
                rValue >>= aEntry;
                if ( !aEntry.isEmpty() )
                    pListbox->append_text( aEntry );
            }
            break;

            case ControlActions::ADD_ITEMS:
            {
                Sequence < OUString > aTemplateList;
                rValue >>= aTemplateList;

                if ( aTemplateList.hasElements() )
                {
                    for ( const OUString& s : std::as_const(aTemplateList) )
                        pListbox->append_text( s );
                }
            }
            break;

            case ControlActions::DELETE_ITEM:
            {
                sal_Int32 nPos = 0;
                if ( rValue >>= nPos )
                    pListbox->remove( nPos );
            }
            break;

            case ControlActions::DELETE_ITEMS:
                pListbox->clear();
                break;

            default:
                SAL_WARN( "fpicker.office", "Wrong ControlAction for implDoListboxAction()" );
        }
    }

    void OControlAccess::implSetControlProperty( sal_Int16 nControlId, weld::Widget* pControl, PropFlags _nProperty, const Any& rValue, bool _bIgnoreIllegalArgument )
    {
        if ( !pControl )
            pControl = m_pFilePickerController->getControl( nControlId );
        DBG_ASSERT( pControl, "OControlAccess::implSetControlProperty: invalid argument, this will crash!" );
        if ( !pControl )
            return;

        DBG_ASSERT( pControl == m_pFilePickerController->getControl( nControlId ),
            "OControlAccess::implSetControlProperty: inconsistent parameters!" );

        switch ( _nProperty )
        {
            case PropFlags::Text:
            {
                OUString sText;
                if (rValue >>= sText)
                {
                    weld::Label* pLabel = dynamic_cast<weld::Label*>(pControl);
                    assert(pLabel);
                    pLabel->set_label(sText);
                }
                else if ( !_bIgnoreIllegalArgument )
                {
                    lcl_throwIllegalArgumentException();
                }
            }
            break;

            case PropFlags::Enabled:
            {
                bool bEnabled = false;
                if ( rValue >>= bEnabled )
                {
                    m_pFilePickerController->enableControl( nControlId, bEnabled );
                }
                else if ( !_bIgnoreIllegalArgument )
                {
                    lcl_throwIllegalArgumentException();
                }
            }
            break;

            case PropFlags::Visible:
            {
                bool bVisible = false;
                if ( rValue >>= bVisible )
                {
                    pControl->set_visible( bVisible );
                }
                else if ( !_bIgnoreIllegalArgument )
                {
                    lcl_throwIllegalArgumentException();
                }
            }
            break;

            case PropFlags::HelpUrl:
            {
                OUString sHelpURL;
                if ( rValue >>= sHelpURL )
                {
                    setHelpURL(pControl, sHelpURL);
                }
                else if ( !_bIgnoreIllegalArgument )
                {
                    lcl_throwIllegalArgumentException();
                }
            }
            break;

            case PropFlags::ListItems:
            {
                weld::ComboBox* pComboBox = dynamic_cast<weld::ComboBox*>(pControl);
                assert(pComboBox && "OControlAccess::implSetControlProperty: invalid control/property combination!");

                Sequence< OUString > aItems;
                if ( rValue >>= aItems )
                {
                    // remove all previous items
                    pComboBox->clear();

                    // add the new ones
                    for (auto const & item : std::as_const(aItems))
                    {
                        pComboBox->append_text(item);
                    }

                }
                else if ( !_bIgnoreIllegalArgument )
                {
                    lcl_throwIllegalArgumentException();
                }
            }
            break;

            case PropFlags::SelectedItem:
            {
                weld::ComboBox* pComboBox = dynamic_cast<weld::ComboBox*>(pControl);
                assert(pComboBox && "OControlAccess::implSetControlProperty: invalid control/property combination!");

                OUString sSelected;
                if ( rValue >>= sSelected )
                {
                    pComboBox->set_active_text(sSelected);
                }
                else if ( !_bIgnoreIllegalArgument )
                {
                    lcl_throwIllegalArgumentException();
                }
            }
            break;

            case PropFlags::SelectedItemIndex:
            {
                weld::ComboBox* pComboBox = dynamic_cast<weld::ComboBox*>(pControl);
                assert(pComboBox && "OControlAccess::implSetControlProperty: invalid control/property combination!");

                sal_Int32 nPos = 0;
                if ( rValue >>= nPos )
                {
                    pComboBox->set_active(nPos);
                }
                else if ( !_bIgnoreIllegalArgument )
                {
                    lcl_throwIllegalArgumentException();
                }
            }
            break;

            case PropFlags::Checked:
            {
                weld::ToggleButton* pToggleButton = dynamic_cast<weld::ToggleButton*>(pControl);
                assert(pToggleButton && "OControlAccess::implSetControlProperty: invalid control/property combination!");

                bool bChecked = false;
                if ( rValue >>= bChecked )
                {
                    pToggleButton->set_active(bChecked);
                }
                else if ( !_bIgnoreIllegalArgument )
                {
                    lcl_throwIllegalArgumentException();
                }
            }
            break;

            default:
                OSL_FAIL( "OControlAccess::implSetControlProperty: invalid property id!" );
        }
    }

    Any OControlAccess::implGetControlProperty( weld::Widget const * pControl, PropFlags _nProperty ) const
    {
        assert(pControl && "OControlAccess::implGetControlProperty: invalid argument, this will crash!");

        Any aReturn;
        switch ( _nProperty )
        {
            case PropFlags::Text:
            {
                const weld::Label* pLabel = dynamic_cast<const weld::Label*>(pControl);
                assert(pLabel);
                aReturn <<= pLabel->get_label();
                break;
            }
            case PropFlags::Enabled:
                aReturn <<= pControl->get_sensitive();
                break;

            case PropFlags::Visible:
                aReturn <<= pControl->get_visible();
                break;

            case PropFlags::HelpUrl:
                aReturn <<= getHelpURL(pControl);
                break;

            case PropFlags::ListItems:
            {
                const weld::ComboBox* pComboBox = dynamic_cast<const weld::ComboBox*>(pControl);
                assert(pComboBox && "OControlAccess::implGetControlProperty: invalid control/property combination!");

                Sequence< OUString > aItems(pComboBox->get_count());
                OUString* pItems = aItems.getArray();
                for (sal_Int32 i = 0; i < pComboBox->get_count(); ++i)
                    *pItems++ = pComboBox->get_text(i);

                aReturn <<= aItems;
                break;
            }

            case PropFlags::SelectedItem:
            {
                const weld::ComboBox* pComboBox = dynamic_cast<const weld::ComboBox*>(pControl);
                assert(pComboBox && "OControlAccess::implGetControlProperty: invalid control/property combination!");

                sal_Int32 nSelected = pComboBox->get_active();
                OUString sSelected;
                if (nSelected != -1)
                    sSelected = pComboBox->get_active_text();
                aReturn <<= sSelected;
                break;
            }

            case PropFlags::SelectedItemIndex:
            {
                const weld::ComboBox* pComboBox = dynamic_cast<const weld::ComboBox*>(pControl);
                assert(pComboBox && "OControlAccess::implGetControlProperty: invalid control/property combination!");

                sal_Int32 nSelected = pComboBox->get_active();
                if (nSelected != -1)
                    aReturn <<= nSelected;
                else
                    aReturn <<= sal_Int32(-1);
                break;
            }

            case PropFlags::Checked:
            {
                const weld::ToggleButton* pToggleButton = dynamic_cast<const weld::ToggleButton*>(pControl);
                assert(pToggleButton && "OControlAccess::implGetControlProperty: invalid control/property combination!");

                aReturn <<= pToggleButton->get_active();
                break;
            }

            default:
                OSL_FAIL( "OControlAccess::implGetControlProperty: invalid property id!" );
        }
        return aReturn;
    }

}   // namespace svt

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
