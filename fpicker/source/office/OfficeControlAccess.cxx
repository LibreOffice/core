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
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <tools/urlobj.hxx>
#include <tools/debug.hxx>

#include <algorithm>
#include <functional>


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
            const sal_Char* pControlName;
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
            bool operator()( const ControlDescription& _rDesc1, const ControlDescription& _rDesc2 )
            {
                return strcmp(_rDesc1.pControlName, _rDesc2.pControlName) < 0;
            }
        };


        struct ControlProperty
        {
            const sal_Char* pPropertyName;
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


    OControlAccess::OControlAccess( IFilePickerController* _pController, SvtFileView* _pFileView )
        :m_pFilePickerController( _pController )
        ,m_pFileView( _pFileView )
    {
        DBG_ASSERT( m_pFilePickerController, "OControlAccess::OControlAccess: invalid control locator!" );
    }


    void OControlAccess::setHelpURL( vcl::Window* _pControl, const OUString& sHelpURL, bool _bFileView )
    {
        OUString sHelpID( sHelpURL );
        INetURLObject aHID( sHelpURL );
        if ( aHID.GetProtocol() == INetProtocol::Hid )
              sHelpID = aHID.GetURLPath();

        // URLs should always be UTF8 encoded and escaped
        OString sID( OUStringToOString( sHelpID, RTL_TEXTENCODING_UTF8 ) );
        if ( _bFileView )
            // the file view "overrides" the SetHelpId
            static_cast< SvtFileView* >( _pControl )->SetHelpId( sID );
        else
            _pControl->SetHelpId( sID );
    }


    OUString OControlAccess::getHelpURL( vcl::Window const * _pControl, bool _bFileView )
    {
        OString aHelpId = _pControl->GetHelpId();
        if ( _bFileView )
            // the file view "overrides" the SetHelpId
            aHelpId = static_cast< SvtFileView const * >( _pControl )->GetHelpId( );

        OUString sHelpURL;
        OUString aTmp( OStringToOUString( aHelpId, RTL_TEXTENCODING_UTF8 ) );
        INetURLObject aHID( aTmp );
        if ( aHID.GetProtocol() == INetProtocol::NotValid )
            sHelpURL = INET_HID_SCHEME;
        sHelpURL += aTmp;
        return sHelpURL;
    }


    Any OControlAccess::getControlProperty( const OUString& _rControlName, const OUString& _rControlProperty )
    {
        // look up the control
        sal_Int16 nControlId = -1;
        PropFlags nPropertyMask = PropFlags::NONE;
        Control* pControl = implGetControl( _rControlName, &nControlId, &nPropertyMask );
            // will throw an IllegalArgumentException if the name is not valid

        // look up the property
        ControlPropertyIterator aPropDesc = ::std::find_if( s_pProperties, s_pPropertiesEnd, ControlPropertyLookup( _rControlProperty ) );
        if ( aPropDesc == s_pPropertiesEnd )
            // it's a completely unknown property
            lcl_throwIllegalArgumentException();

        if ( !( nPropertyMask & aPropDesc->nPropertyId ) )
            // it's a property which is known, but not allowed for this control
            lcl_throwIllegalArgumentException();

        return implGetControlProperty( pControl, aPropDesc->nPropertyId );
    }


    Control* OControlAccess::implGetControl( const OUString& _rControlName, sal_Int16* _pId, PropFlags* _pPropertyMask ) const
    {
        Control* pControl = nullptr;
        ControlDescription tmpDesc;
        OString aControlName = OUStringToOString( _rControlName, RTL_TEXTENCODING_UTF8 );
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


    void OControlAccess::setControlProperty( const OUString& _rControlName, const OUString& _rControlProperty, const css::uno::Any& _rValue )
    {
        // look up the control
        sal_Int16 nControlId = -1;
        Control* pControl = implGetControl( _rControlName, &nControlId );
            // will throw an IllegalArgumentException if the name is not valid

        // look up the property
        ControlPropertyIterator aPropDesc = ::std::find_if( s_pProperties, s_pPropertiesEnd, ControlPropertyLookup( _rControlProperty ) );
        if ( aPropDesc == s_pPropertiesEnd )
            lcl_throwIllegalArgumentException();

        // set the property
        implSetControlProperty( nControlId, pControl, aPropDesc->nPropertyId, _rValue, false );
    }


    Sequence< OUString > OControlAccess::getSupportedControls(  )
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


    Sequence< OUString > OControlAccess::getSupportedControlProperties( const OUString& _rControlName )
    {
        sal_Int16 nControlId = -1;
        PropFlags nPropertyMask = PropFlags::NONE;
        implGetControl( _rControlName, &nControlId, &nPropertyMask );
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


    bool OControlAccess::isControlSupported( const OUString& _rControlName )
    {
        ControlDescription tmpDesc;
        OString aControlName = OUStringToOString(_rControlName, RTL_TEXTENCODING_UTF8);
        tmpDesc.pControlName = aControlName.getStr();
        return ::std::binary_search( s_pControls, s_pControlsEnd, tmpDesc, ControlDescriptionLookup() );
    }


    bool OControlAccess::isControlPropertySupported( const OUString& _rControlName, const OUString& _rControlProperty )
    {
        // look up the control
        sal_Int16 nControlId = -1;
        PropFlags nPropertyMask = PropFlags::NONE;
        implGetControl( _rControlName, &nControlId, &nPropertyMask );
            // will throw an IllegalArgumentException if the name is not valid

        // look up the property
        ControlPropertyIterator aPropDesc = ::std::find_if( s_pProperties, s_pPropertiesEnd, ControlPropertyLookup( _rControlProperty ) );
        if ( aPropDesc == s_pPropertiesEnd )
            // it's a property which is completely unknown
            return false;

        return bool( aPropDesc->nPropertyId & nPropertyMask );
    }


    void OControlAccess::setValue( sal_Int16 _nControlId, sal_Int16 _nControlAction, const Any& _rValue )
    {
        Control* pControl = m_pFilePickerController->getControl( _nControlId );
        DBG_ASSERT( pControl, "OControlAccess::SetValue: don't have this control in the current mode!" );
        if ( pControl )
        {
            PropFlags nPropertyId = PropFlags::Unknown;
            if ( ControlActions::SET_HELP_URL == _nControlAction )
            {
                nPropertyId = PropFlags::HelpUrl;
            }
            else
            {
                switch ( _nControlId )
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
                        if ( ControlActions::SET_SELECT_ITEM == _nControlAction )
                        {
                            nPropertyId = PropFlags::SelectedItemIndex;
                        }
                        else
                        {
                            DBG_ASSERT( WindowType::LISTBOX == pControl->GetType(), "OControlAccess::SetValue: implGetControl returned nonsense!" );
                            implDoListboxAction( static_cast< ListBox* >( pControl ), _nControlAction, _rValue );
                        }
                        break;
                }
            }

            if ( PropFlags::Unknown != nPropertyId )
                implSetControlProperty( _nControlId, pControl, nPropertyId, _rValue );
        }
    }


    Any OControlAccess::getValue( sal_Int16 _nControlId, sal_Int16 _nControlAction ) const
    {
        Any aRet;

        Control* pControl = m_pFilePickerController->getControl( _nControlId );
        DBG_ASSERT( pControl, "OControlAccess::GetValue: don't have this control in the current mode!" );
        if ( pControl )
        {
            PropFlags nPropertyId = PropFlags::Unknown;
            if ( ControlActions::SET_HELP_URL == _nControlAction )
            {
                nPropertyId = PropFlags::HelpUrl;
            }
            else
            {
                switch ( _nControlId )
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
                        if ( ControlActions::GET_SELECTED_ITEM == _nControlAction )
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
                        switch ( _nControlAction )
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
        Control* pControl = m_pFilePickerController->getControl( nId, true );
        DBG_ASSERT( pControl, "OControlAccess::GetValue: don't have this control in the current mode!" );
        if ( pControl )
            pControl->SetText( rLabel );
    }


    OUString OControlAccess::getLabel( sal_Int16 nId ) const
    {
        OUString sLabel;

        Control* pControl = m_pFilePickerController->getControl( nId, true );
        DBG_ASSERT( pControl, "OControlAccess::GetValue: don't have this control in the current mode!" );
        if ( pControl )
            sLabel = pControl->GetText();

        return sLabel;
    }


    void OControlAccess::enableControl( sal_Int16 _nId, bool _bEnable )
    {
        m_pFilePickerController->enableControl( _nId, _bEnable );
    }


    void OControlAccess::implDoListboxAction( ListBox* _pListbox, sal_Int16 _nControlAction, const Any& _rValue )
    {
        switch ( _nControlAction )
        {
            case ControlActions::ADD_ITEM:
            {
                OUString aEntry;
                _rValue >>= aEntry;
                if ( !aEntry.isEmpty() )
                    _pListbox->InsertEntry( aEntry );
            }
            break;

            case ControlActions::ADD_ITEMS:
            {
                Sequence < OUString > aTemplateList;
                _rValue >>= aTemplateList;

                if ( aTemplateList.getLength() )
                {
                    for ( long i=0; i < aTemplateList.getLength(); i++ )
                        _pListbox->InsertEntry( aTemplateList[i] );
                }
            }
            break;

            case ControlActions::DELETE_ITEM:
            {
                sal_Int32 nPos = 0;
                if ( _rValue >>= nPos )
                    _pListbox->RemoveEntry( nPos );
            }
            break;

            case ControlActions::DELETE_ITEMS:
                _pListbox->Clear();
                break;

            default:
                SAL_WARN( "fpicker.office", "Wrong ControlAction for implDoListboxAction()" );
        }
    }


    void OControlAccess::implSetControlProperty( sal_Int16 _nControlId, Control* _pControl, PropFlags _nProperty, const Any& _rValue, bool _bIgnoreIllegalArgument )
    {
        if ( !_pControl )
            _pControl = m_pFilePickerController->getControl( _nControlId );
        DBG_ASSERT( _pControl, "OControlAccess::implSetControlProperty: invalid argument, this will crash!" );
        if ( !_pControl )
            return;

        DBG_ASSERT( _pControl == m_pFilePickerController->getControl( _nControlId ),
            "OControlAccess::implSetControlProperty: inconsistent parameters!" );

        switch ( _nProperty )
        {
            case PropFlags::Text:
            {
                OUString sText;
                if ( _rValue >>= sText )
                {
                    _pControl->SetText( sText );
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
                if ( _rValue >>= bEnabled )
                {
                    m_pFilePickerController->enableControl( _nControlId, bEnabled );
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
                if ( _rValue >>= bVisible )
                {
                    _pControl->Show( bVisible );
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
                if ( _rValue >>= sHelpURL )
                {
                    setHelpURL( _pControl, sHelpURL, m_pFileView == _pControl );
                }
                else if ( !_bIgnoreIllegalArgument )
                {
                    lcl_throwIllegalArgumentException();
                }
            }
            break;

            case PropFlags::ListItems:
            {
                DBG_ASSERT( WindowType::LISTBOX == _pControl->GetType(),
                    "OControlAccess::implSetControlProperty: invalid control/property combination!" );

                Sequence< OUString > aItems;
                if ( _rValue >>= aItems )
                {
                    // remove all previous items
                    static_cast< ListBox* >( _pControl )->Clear();

                    // add the new ones
                    for ( auto const & item : aItems )
                    {
                        static_cast< ListBox* >( _pControl )->InsertEntry( item );
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
                DBG_ASSERT( WindowType::LISTBOX == _pControl->GetType(),
                    "OControlAccess::implSetControlProperty: invalid control/property combination!" );

                OUString sSelected;
                if ( _rValue >>= sSelected )
                {
                    static_cast< ListBox* >( _pControl )->SelectEntry( sSelected );
                }
                else if ( !_bIgnoreIllegalArgument )
                {
                    lcl_throwIllegalArgumentException();
                }
            }
            break;

            case PropFlags::SelectedItemIndex:
            {
                DBG_ASSERT( WindowType::LISTBOX == _pControl->GetType(),
                    "OControlAccess::implSetControlProperty: invalid control/property combination!" );

                sal_Int32 nPos = 0;
                if ( _rValue >>= nPos )
                {
                    static_cast< ListBox* >( _pControl )->SelectEntryPos( nPos );
                }
                else if ( !_bIgnoreIllegalArgument )
                {
                    lcl_throwIllegalArgumentException();
                }
            }
            break;

            case PropFlags::Checked:
            {
                DBG_ASSERT( WindowType::CHECKBOX == _pControl->GetType(),
                    "OControlAccess::implSetControlProperty: invalid control/property combination!" );

                bool bChecked = false;
                if ( _rValue >>= bChecked )
                {
                    static_cast< CheckBox* >( _pControl )->Check( bChecked );
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


    Any OControlAccess::implGetControlProperty( Control const * _pControl, PropFlags _nProperty ) const
    {
        DBG_ASSERT( _pControl, "OControlAccess::implGetControlProperty: invalid argument, this will crash!" );

        Any aReturn;
        switch ( _nProperty )
        {
            case PropFlags::Text:
                aReturn <<= _pControl->GetText();
                break;

            case PropFlags::Enabled:
                aReturn <<= _pControl->IsEnabled();
                break;

            case PropFlags::Visible:
                aReturn <<= _pControl->IsVisible();
                break;

            case PropFlags::HelpUrl:
                aReturn <<= getHelpURL( _pControl, m_pFileView == _pControl );
                break;

            case PropFlags::ListItems:
            {
                DBG_ASSERT( WindowType::LISTBOX == _pControl->GetType(),
                    "OControlAccess::implGetControlProperty: invalid control/property combination!" );

                Sequence< OUString > aItems( static_cast< ListBox const * >( _pControl )->GetEntryCount() );
                OUString* pItems = aItems.getArray();
                for ( sal_Int32 i=0; i<static_cast< ListBox const * >( _pControl )->GetEntryCount(); ++i )
                    *pItems++ = static_cast< ListBox const * >( _pControl )->GetEntry( i );

                aReturn <<= aItems;
            }
            break;

            case PropFlags::SelectedItem:
            {
                DBG_ASSERT( WindowType::LISTBOX == _pControl->GetType(),
                    "OControlAccess::implGetControlProperty: invalid control/property combination!" );

                sal_Int32 nSelected = static_cast< ListBox const * >( _pControl )->GetSelectedEntryPos();
                OUString sSelected;
                if ( LISTBOX_ENTRY_NOTFOUND != nSelected )
                    sSelected = static_cast< ListBox const * >( _pControl )->GetSelectedEntry();
                aReturn <<= sSelected;
            }
            break;

            case PropFlags::SelectedItemIndex:
            {
                DBG_ASSERT( WindowType::LISTBOX == _pControl->GetType(),
                    "OControlAccess::implGetControlProperty: invalid control/property combination!" );

                sal_Int32 nSelected = static_cast< ListBox const * >( _pControl )->GetSelectedEntryPos();
                if ( LISTBOX_ENTRY_NOTFOUND != nSelected )
                    aReturn <<= static_cast< ListBox const * >( _pControl )->GetSelectedEntryPos();
                else
                    aReturn <<= sal_Int32(-1);
            }
            break;

            case PropFlags::Checked:
                DBG_ASSERT( WindowType::CHECKBOX == _pControl->GetType(),
                    "OControlAccess::implGetControlProperty: invalid control/property combination!" );

                aReturn <<= static_cast< CheckBox const * >( _pControl )->IsChecked( );
                break;

            default:
                OSL_FAIL( "OControlAccess::implGetControlProperty: invalid property id!" );
        }
        return aReturn;
    }


}   // namespace svt


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
