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
#include <vcl/lstbox.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <tools/urlobj.hxx>

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

        #define PROPERTY_FLAG_TEXT                  0x00000001
        #define PROPERTY_FLAG_ENDBALED              0x00000002
        #define PROPERTY_FLAG_VISIBLE               0x00000004
        #define PROPERTY_FLAG_HELPURL               0x00000008
        #define PROPERTY_FLAG_LISTITEMS             0x00000010
        #define PROPERTY_FLAG_SELECTEDITEM          0x00000020
        #define PROPERTY_FLAG_SELECTEDITEMINDEX     0x00000040
        #define PROPERTY_FLAG_CHECKED               0x00000080



        struct ControlDescription
        {
            const sal_Char* pControlName;
            sal_Int16       nControlId;
            sal_Int32       nPropertyFlags;
        };


        typedef const ControlDescription* ControlDescIterator;
        typedef ::std::pair< ControlDescIterator, ControlDescIterator > ControlDescRange;


        #define PROPERTY_FLAGS_COMMON       ( PROPERTY_FLAG_ENDBALED | PROPERTY_FLAG_VISIBLE | PROPERTY_FLAG_HELPURL )
        #define PROPERTY_FLAGS_LISTBOX      ( PROPERTY_FLAG_LISTITEMS | PROPERTY_FLAG_SELECTEDITEM | PROPERTY_FLAG_SELECTEDITEMINDEX )
        #define PROPERTY_FLAGS_CHECKBOX     ( PROPERTY_FLAG_CHECKED | PROPERTY_FLAG_TEXT )

        // Note: this array MUST be sorted by name!
        static const ControlDescription aDescriptions[] =  {
            { "AutoExtensionBox",       CHECKBOX_AUTOEXTENSION,         PROPERTY_FLAGS_COMMON | PROPERTY_FLAGS_CHECKBOX     },
            { "CancelButton",           PUSHBUTTON_CANCEL,              PROPERTY_FLAGS_COMMON | PROPERTY_FLAG_TEXT          },
            { "CurrentFolderText",      FIXEDTEXT_CURRENTFOLDER,        PROPERTY_FLAGS_COMMON | PROPERTY_FLAG_TEXT          },
            { "DefaultLocationButton",  TOOLBOXBUTOON_DEFAULT_LOCATION, PROPERTY_FLAGS_COMMON                               },
            { "FileURLEdit",            EDIT_FILEURL,                   PROPERTY_FLAGS_COMMON | PROPERTY_FLAG_TEXT          },
            { "FileURLEditLabel",       EDIT_FILEURL_LABEL,             PROPERTY_FLAGS_COMMON | PROPERTY_FLAG_TEXT          },
            { "FileView",               CONTROL_FILEVIEW,               PROPERTY_FLAGS_COMMON                               },
            { "FilterList",             LISTBOX_FILTER,                 PROPERTY_FLAGS_COMMON                               },
            { "FilterListLabel",        LISTBOX_FILTER_LABEL,           PROPERTY_FLAGS_COMMON | PROPERTY_FLAG_TEXT          },
            { "FilterOptionsBox",       CHECKBOX_FILTEROPTIONS,         PROPERTY_FLAGS_COMMON | PROPERTY_FLAGS_CHECKBOX     },
            { "HelpButton",             PUSHBUTTON_HELP,                PROPERTY_FLAGS_COMMON | PROPERTY_FLAG_TEXT          },
            { "ImageTemplateList",      LISTBOX_IMAGE_TEMPLATE,         PROPERTY_FLAGS_COMMON | PROPERTY_FLAGS_LISTBOX      },
            { "ImageTemplateListLabel", LISTBOX_IMAGE_TEMPLATE_LABEL,   PROPERTY_FLAGS_COMMON | PROPERTY_FLAG_TEXT          },
            { "LevelUpButton",          TOOLBOXBUTOON_LEVEL_UP,         PROPERTY_FLAGS_COMMON                               },
            { "LinkBox",                CHECKBOX_LINK,                  PROPERTY_FLAGS_COMMON | PROPERTY_FLAGS_CHECKBOX     },
            { "NewFolderButton",        TOOLBOXBUTOON_NEW_FOLDER,       PROPERTY_FLAGS_COMMON                               },
            { "OkButton",               PUSHBUTTON_OK ,                 PROPERTY_FLAGS_COMMON | PROPERTY_FLAG_TEXT          },
            { "PasswordBox",            CHECKBOX_PASSWORD,              PROPERTY_FLAGS_COMMON | PROPERTY_FLAGS_CHECKBOX     },
            { "PlayButton",             PUSHBUTTON_PLAY,                PROPERTY_FLAGS_COMMON | PROPERTY_FLAG_TEXT          },
            { "PreviewBox",             CHECKBOX_PREVIEW,               PROPERTY_FLAGS_COMMON | PROPERTY_FLAGS_CHECKBOX     },
            { "ReadOnlyBox",            CHECKBOX_READONLY,              PROPERTY_FLAGS_COMMON | PROPERTY_FLAGS_CHECKBOX     },
            { "SelectionBox",           CHECKBOX_SELECTION,             PROPERTY_FLAGS_COMMON | PROPERTY_FLAGS_CHECKBOX     },
            { "TemplateList",           LISTBOX_TEMPLATE,               PROPERTY_FLAGS_COMMON | PROPERTY_FLAGS_LISTBOX      },
            { "TemplateListLabel",      LISTBOX_TEMPLATE_LABEL,         PROPERTY_FLAGS_COMMON | PROPERTY_FLAG_TEXT          },
            { "VersionList",            LISTBOX_VERSION,                PROPERTY_FLAGS_COMMON | PROPERTY_FLAGS_LISTBOX      },
            { "VersionListLabel",       LISTBOX_VERSION_LABEL,          PROPERTY_FLAGS_COMMON | PROPERTY_FLAG_TEXT          }
        };


        static const sal_Int32 s_nControlCount = sizeof( aDescriptions ) / sizeof( aDescriptions[0] );

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
            sal_Int16       nPropertyId;
        };

        typedef const ControlProperty* ControlPropertyIterator;


        static const ControlProperty aProperties[] =  {
            { "Text",               PROPERTY_FLAG_TEXT              },
            { "Enabled",            PROPERTY_FLAG_ENDBALED          },
            { "Visible",            PROPERTY_FLAG_VISIBLE           },
            { "HelpURL",            PROPERTY_FLAG_HELPURL           },
            { "ListItems",          PROPERTY_FLAG_LISTITEMS         },
            { "SelectedItem",       PROPERTY_FLAG_SELECTEDITEM      },
            { "SelectedItemIndex",  PROPERTY_FLAG_SELECTEDITEMINDEX },
            { "Checked",            PROPERTY_FLAG_CHECKED           }
        };


        static const int s_nPropertyCount = sizeof( aProperties ) / sizeof( aProperties[0] );

        static ControlPropertyIterator s_pProperties = aProperties;
        static ControlPropertyIterator s_pPropertiesEnd = aProperties + s_nPropertyCount;


        struct ControlPropertyLookup
        {
            OUString m_sLookup;
            ControlPropertyLookup( const OUString& _rLookup ) : m_sLookup( _rLookup ) { }

            bool operator()( const ControlProperty& _rProp )
            {
                return m_sLookup.equalsAscii( _rProp.pPropertyName );
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


    OUString OControlAccess::getHelpURL( vcl::Window* _pControl, bool _bFileView )
    {
        OString aHelpId = _pControl->GetHelpId();
        if ( _bFileView )
            // the file view "overrides" the SetHelpId
            aHelpId = static_cast< SvtFileView* >( _pControl )->GetHelpId( );

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
        sal_Int32 nPropertyMask = 0;
        Control* pControl = implGetControl( _rControlName, &nControlId, &nPropertyMask );
            // will throw an IllegalArgumentException if the name is not valid

        // look up the property
        ControlPropertyIterator aPropDesc = ::std::find_if( s_pProperties, s_pPropertiesEnd, ControlPropertyLookup( _rControlProperty ) );
        if ( aPropDesc == s_pPropertiesEnd )
            // it's a completely unknown property
            lcl_throwIllegalArgumentException();

        if ( 0 == ( nPropertyMask & aPropDesc->nPropertyId ) )
            // it's a property which is known, but not allowed for this control
            lcl_throwIllegalArgumentException();

        return implGetControlProperty( pControl, aPropDesc->nPropertyId );
    }


    Control* OControlAccess::implGetControl( const OUString& _rControlName, sal_Int16* _pId, sal_Int32* _pPropertyMask ) const
    {
        Control* pControl = nullptr;
        ControlDescription tmpDesc;
        OString aControlName = OUStringToOString( _rControlName, RTL_TEXTENCODING_UTF8 );
        tmpDesc.pControlName = aControlName.getStr();

        // translate the name into an id
        ControlDescRange aFoundRange = ::std::equal_range( s_pControls, s_pControlsEnd, tmpDesc, ControlDescriptionLookup() );
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
        sal_Int32 nPropertyMask = 0;
        implGetControl( _rControlName, &nControlId, &nPropertyMask );
            // will throw an IllegalArgumentException if the name is not valid

        // fill in the property names
        Sequence< OUString > aProps( s_nPropertyCount );
        OUString* pProperty = aProps.getArray();

        for ( ControlPropertyIterator aProp = s_pProperties; aProp != s_pPropertiesEnd; ++aProp )
            if ( 0 != ( nPropertyMask & aProp->nPropertyId ) )
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
        sal_Int32 nPropertyMask = 0;
        implGetControl( _rControlName, &nControlId, &nPropertyMask );
            // will throw an IllegalArgumentException if the name is not valid

        // look up the property
        ControlPropertyIterator aPropDesc = ::std::find_if( s_pProperties, s_pPropertiesEnd, ControlPropertyLookup( _rControlProperty ) );
        if ( aPropDesc == s_pPropertiesEnd )
            // it's a property which is completely unknown
            return false;

        return 0 != ( aPropDesc->nPropertyId & nPropertyMask );
    }


    void OControlAccess::setValue( sal_Int16 _nControlId, sal_Int16 _nControlAction, const Any& _rValue )
    {
        Control* pControl = m_pFilePickerController->getControl( _nControlId );
        DBG_ASSERT( pControl, "OControlAccess::SetValue: don't have this control in the current mode!" );
        if ( pControl )
        {
            sal_Int16 nPropertyId = -1;
            if ( ControlActions::SET_HELP_URL == _nControlAction )
            {
                nPropertyId = PROPERTY_FLAG_HELPURL;
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
                        nPropertyId = PROPERTY_FLAG_CHECKED;
                        break;

                    case LISTBOX_FILTER:
                        SAL_WARN( "fpicker.office", "Use the XFilterManager to access the filter listbox" );
                        break;

                    case LISTBOX_VERSION:
                    case LISTBOX_TEMPLATE:
                    case LISTBOX_IMAGE_TEMPLATE:
                        if ( ControlActions::SET_SELECT_ITEM == _nControlAction )
                        {
                            nPropertyId = PROPERTY_FLAG_SELECTEDITEMINDEX;
                        }
                        else
                        {
                            DBG_ASSERT( WINDOW_LISTBOX == pControl->GetType(), "OControlAccess::SetValue: implGetControl returned nonsense!" );
                            implDoListboxAction( static_cast< ListBox* >( pControl ), _nControlAction, _rValue );
                        }
                        break;
                }
            }

            if ( -1 != nPropertyId )
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
            sal_Int16 nPropertyId = -1;
            if ( ControlActions::SET_HELP_URL == _nControlAction )
            {
                nPropertyId = PROPERTY_FLAG_HELPURL;
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
                        nPropertyId = PROPERTY_FLAG_CHECKED;
                        break;

                    case LISTBOX_FILTER:
                        if ( ControlActions::GET_SELECTED_ITEM == _nControlAction )
                        {
                            aRet <<= OUString( m_pFilePickerController->getCurFilter() );;
                        }
                        else
                        {
                            SAL_WARN( "fpicker.office", "Use the XFilterManager to access the filter listbox" );
                        }
                        break;

                    case LISTBOX_VERSION:
                    case LISTBOX_TEMPLATE:
                    case LISTBOX_IMAGE_TEMPLATE:
                        switch ( _nControlAction )
                        {
                            case ControlActions::GET_SELECTED_ITEM:
                                nPropertyId = PROPERTY_FLAG_SELECTEDITEM;
                                break;
                            case ControlActions::GET_SELECTED_ITEM_INDEX:
                                nPropertyId = PROPERTY_FLAG_SELECTEDITEMINDEX;
                                break;
                            case ControlActions::GET_ITEMS:
                                nPropertyId = PROPERTY_FLAG_LISTITEMS;
                                break;
                            default:
                                SAL_WARN( "fpicker.office", "OControlAccess::GetValue: invalid control action for the listbox!" );
                                break;
                        }
                        break;
                }
            }

            if ( -1 != nPropertyId )
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


    void OControlAccess::implSetControlProperty( sal_Int16 _nControlId, Control* _pControl, sal_Int16 _nProperty, const Any& _rValue, bool _bIgnoreIllegalArgument )
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
            case PROPERTY_FLAG_TEXT:
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

            case PROPERTY_FLAG_ENDBALED:
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

            case PROPERTY_FLAG_VISIBLE:
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

            case PROPERTY_FLAG_HELPURL:
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

            case PROPERTY_FLAG_LISTITEMS:
            {
                DBG_ASSERT( WINDOW_LISTBOX == _pControl->GetType(),
                    "OControlAccess::implSetControlProperty: invalid control/property combination!" );

                Sequence< OUString > aItems;
                if ( _rValue >>= aItems )
                {
                    // remove all previous items
                    static_cast< ListBox* >( _pControl )->Clear();

                    // add the new ones
                    const OUString* pItems       = aItems.getConstArray();
                    const OUString* pItemsEnd    = aItems.getConstArray() + aItems.getLength();
                    for (   const OUString* pItem = pItems;
                            pItem != pItemsEnd;
                            ++pItem
                        )
                    {
                        static_cast< ListBox* >( _pControl )->InsertEntry( *pItem );
                    }

                }
                else if ( !_bIgnoreIllegalArgument )
                {
                    lcl_throwIllegalArgumentException();
                }
            }
            break;

            case PROPERTY_FLAG_SELECTEDITEM:
            {
                DBG_ASSERT( WINDOW_LISTBOX == _pControl->GetType(),
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

            case PROPERTY_FLAG_SELECTEDITEMINDEX:
            {
                DBG_ASSERT( WINDOW_LISTBOX == _pControl->GetType(),
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

            case PROPERTY_FLAG_CHECKED:
            {
                DBG_ASSERT( WINDOW_CHECKBOX == _pControl->GetType(),
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


    Any OControlAccess::implGetControlProperty( Control* _pControl, sal_Int16 _nProperty ) const
    {
        DBG_ASSERT( _pControl, "OControlAccess::implGetControlProperty: invalid argument, this will crash!" );

        Any aReturn;
        switch ( _nProperty )
        {
            case PROPERTY_FLAG_TEXT:
                aReturn <<= OUString( _pControl->GetText() );
                break;

            case PROPERTY_FLAG_ENDBALED:
                aReturn <<= _pControl->IsEnabled();
                break;

            case PROPERTY_FLAG_VISIBLE:
                aReturn <<= _pControl->IsVisible();
                break;

            case PROPERTY_FLAG_HELPURL:
                aReturn <<= getHelpURL( _pControl, m_pFileView == _pControl );
                break;

            case PROPERTY_FLAG_LISTITEMS:
            {
                DBG_ASSERT( WINDOW_LISTBOX == _pControl->GetType(),
                    "OControlAccess::implGetControlProperty: invalid control/property combination!" );

                Sequence< OUString > aItems( static_cast< ListBox* >( _pControl )->GetEntryCount() );
                OUString* pItems = aItems.getArray();
                for ( sal_Int32 i=0; i<static_cast< ListBox* >( _pControl )->GetEntryCount(); ++i )
                    *pItems++ = static_cast< ListBox* >( _pControl )->GetEntry( i );

                aReturn <<= aItems;
            }
            break;

            case PROPERTY_FLAG_SELECTEDITEM:
            {
                DBG_ASSERT( WINDOW_LISTBOX == _pControl->GetType(),
                    "OControlAccess::implGetControlProperty: invalid control/property combination!" );

                sal_Int32 nSelected = static_cast< ListBox* >( _pControl )->GetSelectEntryPos();
                OUString sSelected;
                if ( LISTBOX_ENTRY_NOTFOUND != nSelected )
                    sSelected = static_cast< ListBox* >( _pControl )->GetSelectEntry();
                aReturn <<= sSelected;
            }
            break;

            case PROPERTY_FLAG_SELECTEDITEMINDEX:
            {
                DBG_ASSERT( WINDOW_LISTBOX == _pControl->GetType(),
                    "OControlAccess::implGetControlProperty: invalid control/property combination!" );

                sal_Int32 nSelected = static_cast< ListBox* >( _pControl )->GetSelectEntryPos();
                if ( LISTBOX_ENTRY_NOTFOUND != nSelected )
                    aReturn <<= (sal_Int32)static_cast< ListBox* >( _pControl )->GetSelectEntryPos();
                else
                    aReturn <<= (sal_Int32)-1;
            }
            break;

            case PROPERTY_FLAG_CHECKED:
                DBG_ASSERT( WINDOW_CHECKBOX == _pControl->GetType(),
                    "OControlAccess::implGetControlProperty: invalid control/property combination!" );

                aReturn <<= static_cast< CheckBox* >( _pControl )->IsChecked( );
                break;

            default:
                OSL_FAIL( "OControlAccess::implGetControlProperty: invalid property id!" );
        }
        return aReturn;
    }


}   // namespace svt


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
