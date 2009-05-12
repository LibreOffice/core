/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: richtextunowrapper.cxx,v $
 * $Revision: 1.6 $
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
#include "precompiled_forms.hxx"
#include "richtextunowrapper.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/container/XNameContainer.hpp>
/** === end UNO includes === **/
#include <svx/unofored.hxx>
#include <svx/editview.hxx>

//........................................................................
namespace frm
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;

    //====================================================================
    namespace
    {
        const SfxItemPropertyMap* getTextEnginePropertyMap()
        {
            // Propertymap fuer einen Outliner Text
            static const SfxItemPropertyMap aTextEnginePropertyMap[] =
            {
                SVX_UNOEDIT_CHAR_PROPERTIES,
                SVX_UNOEDIT_FONT_PROPERTIES,
                SVX_UNOEDIT_PARA_PROPERTIES,
                { MAP_CHAR_LEN("TextUserDefinedAttributes"), EE_CHAR_XMLATTRIBS, &::getCppuType( static_cast< const Reference< XNameContainer >* >( NULL ) ), 0, 0 },
                { MAP_CHAR_LEN("ParaUserDefinedAttributes"), EE_PARA_XMLATTRIBS, &::getCppuType( static_cast< const Reference< XNameContainer >* >( NULL ) ), 0, 0 },
                { NULL, 0, 0, NULL, 0, 0 }
            };

            return aTextEnginePropertyMap;
        }
    }

    //====================================================================
    //= ORichTextUnoWrapper
    //====================================================================
    //--------------------------------------------------------------------
    ORichTextUnoWrapper::ORichTextUnoWrapper( EditEngine& _rEngine, IEngineTextChangeListener* _pTextChangeListener )
        :SvxUnoText( getTextEnginePropertyMap() )
    {
        SetEditSource( new RichTextEditSource( _rEngine, _pTextChangeListener ) );
    }

    //--------------------------------------------------------------------
    ORichTextUnoWrapper::~ORichTextUnoWrapper() throw()
    {
    }

    //====================================================================
    //= RichTextEditSource
    //====================================================================
    //--------------------------------------------------------------------
    RichTextEditSource::RichTextEditSource( EditEngine& _rEngine, IEngineTextChangeListener* _pTextChangeListener )
        :m_rEngine              ( _rEngine                               )
        ,m_pTextForwarder       ( new SvxEditEngineForwarder( _rEngine ) )
        ,m_pTextChangeListener  ( _pTextChangeListener                   )
    {
    }

    //--------------------------------------------------------------------
    RichTextEditSource::~RichTextEditSource()
    {
        delete m_pTextForwarder;
    }

    //--------------------------------------------------------------------
    SvxEditSource* RichTextEditSource::Clone() const
    {
        return new RichTextEditSource( m_rEngine, m_pTextChangeListener );
    }

    //--------------------------------------------------------------------
    SvxTextForwarder* RichTextEditSource::GetTextForwarder()
    {
        return m_pTextForwarder;
    }

    //--------------------------------------------------------------------
    void RichTextEditSource::UpdateData()
    {
        // this means that the content of the EditEngine changed via the UNO API
        // to reflect this in the views, we need to update them
        USHORT viewCount = m_rEngine.GetViewCount();
        for ( USHORT view = 0; view < viewCount; ++view )
        {
            EditView* pView = m_rEngine.GetView( view );
            if ( pView )
                pView->ForceUpdate();
        }

        if ( m_pTextChangeListener )
            m_pTextChangeListener->potentialTextChange();
    }

//........................................................................
}  // namespace frm
//........................................................................

