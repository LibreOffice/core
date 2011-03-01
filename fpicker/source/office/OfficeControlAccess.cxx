/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_fpicker.hxx"

#include <sal/macros.h>
#include "OfficeControlAccess.hxx"
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ControlActions.hpp>
#include <vcl/lstbox.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <algorithm>
#include <functional>

//.........................................................................
namespace svt
{
//.........................................................................

    // helper -------------------------------------------------------------

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::ui::dialogs;

    using namespace ExtendedFilePickerElementIds;
    using namespace CommonFilePickerElementIds;
    using namespace InternalFilePickerElementIds;

    // --------------------------------------------------------------------
    namespace
    {
        // ----------------------------------------------------------------
        #define PROPERTY_FLAG_TEXT                  0x00000001
        #define PROPERTY_FLAG_ENDBALED              0x00000002
        #define PROPERTY_FLAG_VISIBLE               0x00000004
        #define PROPERTY_FLAG_HELPURL               0x00000008
        #define PROPERTY_FLAG_LISTITEMS             0x00000010
        #define PROPERTY_FLAG_SELECTEDITEM          0x00000020
        #define PROPERTY_FLAG_SELECTEDITEMINDEX     0x00000040
        #define PROPERTY_FLAG_CHECKED               0x00000080

        // ----------------------------------------------------------------
        // ................................................................
        struct ControlDescription
        {
            const sal_Char* pControlName;
            sal_Int16       nControlId;
            sal_Int32       nPropertyFlags;
        };

        // ................................................................
        typedef const ControlDescription* ControlDescIterator;
        typedef ::std::pair< ControlDescIterator, ControlDescIterator > ControlDescRange;

        // ......................................................................
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

        // ................................................................
        static const sal_Int32 s_nControlCount = SAL_N_ELEMENTS( aDescriptions );

        static ControlDescIterator s_pControls = aDescriptions;
        static ControlDescIterator s_pControlsEnd = aDescriptions + s_nControlCount;

        // ................................................................
        struct ControlDescriptionLookup
        {
            bool operator()( const ::rtl::OUString& _rLookup, const ControlDescription& _rDesc )
            {
                return _rLookup.compareToAscii( _rDesc.pControlName ) < 0;
            }
            bool operator()( const ControlDescription& _rDesc, const ::rtl::OUString& _rLookup )
            {
                return _rLookup.compareToAscii( _rDesc.pControlName ) > 0;
            }
        };

        // ................................................................
        struct ExtractControlName : public ::std::unary_function< ControlDescription, ::rtl::OUString >
        {
            ::rtl::OUString operator()( const ControlDescription& _rDesc )
            {
                return ::rtl::OUString::createFromAscii( _rDesc.pControlName );
            }
        };

        // ----------------------------------------------------------------
        // ................................................................
        struct ControlProperty
        {
            const sal_Char* pPropertyName;
            sal_Int16       nPropertyId;
        };

        typedef const ControlProperty* ControlPropertyIterator;

        // ................................................................
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

        // ................................................................
        static const int s_nPropertyCount = SAL_N_ELEMENTS( aProperties );

        static ControlPropertyIterator s_pProperties = aProperties;
        static ControlPropertyIterator s_pPropertiesEnd = aProperties + s_nPropertyCount;

        // ................................................................
        struct ControlPropertyLookup
        {
            ::rtl::OUString m_sLookup;
            ControlPropertyLookup( const ::rtl::OUString& _rLookup ) : m_sLookup( _rLookup ) { }

            sal_Bool operator()( const ControlProperty& _rProp )
            {
                return m_sLookup.equalsAscii( _rProp.pPropertyName );
            }
        };

        //-----------------------------------------------------------------
        void lcl_throwIllegalArgumentException( ) SAL_THROW( (IllegalArgumentException) )
        {
            throw IllegalArgumentException();
            // TODO: error message in the exception
        }
    }

    //---------------------------------------------------------------------
    OControlAccess::OControlAccess( IFilePickerController* _pController, SvtFileView* _pFileView )
        :m_pFilePickerController( _pController )
        ,m_pFileView( _pFileView )
    {
        DBG_ASSERT( m_pFilePickerController, "OControlAccess::OControlAccess: invalid control locator!" );
    }

    //---------------------------------------------------------------------
    void OControlAccess::setHelpURL( Window* _pControl, const ::rtl::OUString& _rURL, sal_Bool _bFileView )
    {
        String sHelpURL( _rURL );
        if ( COMPARE_EQUAL == sHelpURL.CompareIgnoreCaseToAscii( "HID:", sizeof( "HID:" ) - 1 ) )
        {
            String sID = sHelpURL.Copy( sizeof( "HID:" ) - 1 );
            sal_Int32 nHelpId = sID.ToInt32();

            if ( _bFileView )
                // the file view "overloaded" the SetHelpId
                static_cast< SvtFileView* >( _pControl )->SetHelpId( nHelpId );
            else
                _pControl->SetHelpId( nHelpId );
        }
        else
        {
            DBG_ERRORFILE( "OControlAccess::setHelpURL: unsupported help URL type!" );
        }
    }

    //---------------------------------------------------------------------
    ::rtl::OUString OControlAccess::getHelpURL( Window* _pControl, sal_Bool _bFileView )
    {
        sal_Int32 nHelpId = _pControl->GetHelpId();
        if ( _bFileView )
            // the file view "overloaded" the SetHelpId
            nHelpId = static_cast< SvtFileView* >( _pControl )->GetHelpId( );

        ::rtl::OUString sHelpURL( RTL_CONSTASCII_USTRINGPARAM( "HID:" ) );
        sHelpURL += ::rtl::OUString::valueOf( (sal_Int32)nHelpId );

        return sHelpURL;
    }

    // --------------------------------------------------------------------------
    Any OControlAccess::getControlProperty( const ::rtl::OUString& _rControlName, const ::rtl::OUString& _rControlProperty )
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

    //---------------------------------------------------------------------
    Control* OControlAccess::implGetControl( const ::rtl::OUString& _rControlName, sal_Int16* _pId, sal_Int32* _pPropertyMask ) const SAL_THROW( (IllegalArgumentException) )
    {
        Control* pControl = NULL;

        // translate the name into an id
        ControlDescRange aFoundRange = ::std::equal_range( s_pControls, s_pControlsEnd, _rControlName, ControlDescriptionLookup() );
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

    //---------------------------------------------------------------------
    void OControlAccess::setControlProperty( const ::rtl::OUString& _rControlName, const ::rtl::OUString& _rControlProperty, const ::com::sun::star::uno::Any& _rValue )
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
        implSetControlProperty( nControlId, pControl, aPropDesc->nPropertyId, _rValue, sal_False );
    }

    // --------------------------------------------------------------------------
    Sequence< ::rtl::OUString > OControlAccess::getSupportedControls(  )
    {
        Sequence< ::rtl::OUString > aControls( s_nControlCount );
        ::rtl::OUString* pControls = aControls.getArray();

        // collect the names of all _actually_existent_ controls
        for ( ControlDescIterator aControl = s_pControls; aControl != s_pControlsEnd; ++aControl )
        {
            if ( m_pFilePickerController->getControl( aControl->nControlId ) )
                *pControls++ = ::rtl::OUString::createFromAscii( aControl->pControlName );
        }

        aControls.realloc( pControls - aControls.getArray() );
        return aControls;
    }

    // --------------------------------------------------------------------------
    Sequence< ::rtl::OUString > OControlAccess::getSupportedControlProperties( const ::rtl::OUString& _rControlName )
    {
        sal_Int16 nControlId = -1;
        sal_Int32 nPropertyMask = 0;
        implGetControl( _rControlName, &nControlId, &nPropertyMask );
            // will throw an IllegalArgumentException if the name is not valid

        // fill in the property names
        Sequence< ::rtl::OUString > aProps( s_nPropertyCount );
        ::rtl::OUString* pProperty = aProps.getArray();

        for ( ControlPropertyIterator aProp = s_pProperties; aProp != s_pPropertiesEnd; ++aProp )
            if ( 0 != ( nPropertyMask & aProp->nPropertyId ) )
                *pProperty++ = ::rtl::OUString::createFromAscii( aProp->pPropertyName );

        aProps.realloc( pProperty - aProps.getArray() );
        return aProps;
    }

    // --------------------------------------------------------------------------
    sal_Bool OControlAccess::isControlSupported( const ::rtl::OUString& _rControlName )
    {
        return ::std::binary_search( s_pControls, s_pControlsEnd, _rControlName, ControlDescriptionLookup() );
    }

    // --------------------------------------------------------------------------
    sal_Bool OControlAccess::isControlPropertySupported( const ::rtl::OUString& _rControlName, const ::rtl::OUString& _rControlProperty )
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
            return sal_False;

        return 0 != ( aPropDesc->nPropertyId & nPropertyMask );
    }

    //-----------------------------------------------------------------------------
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
                        DBG_ERRORFILE( "Use the XFilterManager to access the filter listbox" );
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

    //-----------------------------------------------------------------------------
    Any OControlAccess::getValue( sal_Int16 _nControlId, sal_Int16 _nControlAction ) const
    {
        Any aRet;

        Control* pControl = m_pFilePickerController->getControl( _nControlId, sal_False );
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
                            aRet <<= ::rtl::OUString( m_pFilePickerController->getCurFilter() );;
                        }
                        else
                        {
                            DBG_ERRORFILE( "Use the XFilterManager to access the filter listbox" );
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
                                DBG_ERRORFILE( "OControlAccess::GetValue: invalid control action for the listbox!" );
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

    //-----------------------------------------------------------------------------
    void OControlAccess::setLabel( sal_Int16 nId, const ::rtl::OUString &rLabel )
    {
        Control* pControl = m_pFilePickerController->getControl( nId, sal_True );
        DBG_ASSERT( pControl, "OControlAccess::GetValue: don't have this control in the current mode!" );
        if ( pControl )
            pControl->SetText( rLabel );
    }

    //-----------------------------------------------------------------------------
    ::rtl::OUString OControlAccess::getLabel( sal_Int16 nId ) const
    {
        ::rtl::OUString sLabel;

        Control* pControl = m_pFilePickerController->getControl( nId, sal_True );
        DBG_ASSERT( pControl, "OControlAccess::GetValue: don't have this control in the current mode!" );
        if ( pControl )
            sLabel = pControl->GetText();

        return sLabel;
    }

    //-----------------------------------------------------------------------------
    void OControlAccess::enableControl( sal_Int16 _nId, sal_Bool _bEnable )
    {
        m_pFilePickerController->enableControl( _nId, _bEnable );
    }

    // -----------------------------------------------------------------------
    void OControlAccess::implDoListboxAction( ListBox* _pListbox, sal_Int16 _nControlAction, const Any& _rValue )
    {
        switch ( _nControlAction )
        {
            case ControlActions::ADD_ITEM:
            {
                ::rtl::OUString aEntry;
                _rValue >>= aEntry;
                if ( aEntry.getLength() )
                    _pListbox->InsertEntry( aEntry );
            }
            break;

            case ControlActions::ADD_ITEMS:
            {
                Sequence < ::rtl::OUString > aTemplateList;
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
                    _pListbox->RemoveEntry( (USHORT) nPos );
            }
            break;

            case ControlActions::DELETE_ITEMS:
                _pListbox->Clear();
                break;

            default:
                DBG_ERRORFILE( "Wrong ControlAction for implDoListboxAction()" );
        }
    }

    //-----------------------------------------------------------------------------
    void OControlAccess::implSetControlProperty( sal_Int16 _nControlId, Control* _pControl, sal_Int16 _nProperty, const Any& _rValue, sal_Bool _bIgnoreIllegalArgument )
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
                ::rtl::OUString sText;
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
                sal_Bool bEnabled = sal_False;
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
                sal_Bool bVisible = sal_False;
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
                ::rtl::OUString sHelpURL;
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

                Sequence< ::rtl::OUString > aItems;
                if ( _rValue >>= aItems )
                {
                    // remove all previous items
                    static_cast< ListBox* >( _pControl )->Clear();

                    // add the new ones
                    const ::rtl::OUString* pItems       = aItems.getConstArray();
                    const ::rtl::OUString* pItemsEnd    = aItems.getConstArray() + aItems.getLength();
                    for (   const ::rtl::OUString* pItem = pItems;
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

                ::rtl::OUString sSelected;
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
                    static_cast< ListBox* >( _pControl )->SelectEntryPos( (USHORT) nPos );
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

                sal_Bool bChecked = sal_False;
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

    //-----------------------------------------------------------------------------
    Any OControlAccess::implGetControlProperty( Control* _pControl, sal_Int16 _nProperty ) const
    {
        DBG_ASSERT( _pControl, "OControlAccess::implGetControlProperty: invalid argument, this will crash!" );

        Any aReturn;
        switch ( _nProperty )
        {
            case PROPERTY_FLAG_TEXT:
                aReturn <<= ::rtl::OUString( _pControl->GetText() );
                break;

            case PROPERTY_FLAG_ENDBALED:
                aReturn <<= (sal_Bool)_pControl->IsEnabled();
                break;

            case PROPERTY_FLAG_VISIBLE:
                aReturn <<= (sal_Bool)_pControl->IsVisible();
                break;

            case PROPERTY_FLAG_HELPURL:
                aReturn <<= getHelpURL( _pControl, m_pFileView == _pControl );
                break;

            case PROPERTY_FLAG_LISTITEMS:
            {
                DBG_ASSERT( WINDOW_LISTBOX == _pControl->GetType(),
                    "OControlAccess::implGetControlProperty: invalid control/property combination!" );

                Sequence< ::rtl::OUString > aItems( static_cast< ListBox* >( _pControl )->GetEntryCount() );
                ::rtl::OUString* pItems = aItems.getArray();
                for ( USHORT i=0; i<static_cast< ListBox* >( _pControl )->GetEntryCount(); ++i )
                    *pItems++ = static_cast< ListBox* >( _pControl )->GetEntry( i );

                aReturn <<= aItems;
            }
            break;

            case PROPERTY_FLAG_SELECTEDITEM:
            {
                DBG_ASSERT( WINDOW_LISTBOX == _pControl->GetType(),
                    "OControlAccess::implGetControlProperty: invalid control/property combination!" );

                USHORT nSelected = static_cast< ListBox* >( _pControl )->GetSelectEntryPos();
                ::rtl::OUString sSelected;
                if ( LISTBOX_ENTRY_NOTFOUND != nSelected )
                    sSelected = static_cast< ListBox* >( _pControl )->GetSelectEntry();
                aReturn <<= sSelected;
            }
            break;

            case PROPERTY_FLAG_SELECTEDITEMINDEX:
            {
                DBG_ASSERT( WINDOW_LISTBOX == _pControl->GetType(),
                    "OControlAccess::implGetControlProperty: invalid control/property combination!" );

                USHORT nSelected = static_cast< ListBox* >( _pControl )->GetSelectEntryPos();
                if ( LISTBOX_ENTRY_NOTFOUND != nSelected )
                    aReturn <<= (sal_Int32)static_cast< ListBox* >( _pControl )->GetSelectEntryPos();
                else
                    aReturn <<= (sal_Int32)-1;
            }
            break;

            case PROPERTY_FLAG_CHECKED:
                DBG_ASSERT( WINDOW_CHECKBOX == _pControl->GetType(),
                    "OControlAccess::implGetControlProperty: invalid control/property combination!" );

                aReturn <<= (sal_Bool)static_cast< CheckBox* >( _pControl )->IsChecked( );
                break;

            default:
                OSL_FAIL( "OControlAccess::implGetControlProperty: invalid property id!" );
        }
        return aReturn;
    }

//.........................................................................
}   // namespace svt
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
