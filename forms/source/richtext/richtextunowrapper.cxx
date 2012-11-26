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
#include "precompiled_forms.hxx"
#include "richtextunowrapper.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/container/XNameContainer.hpp>
/** === end UNO includes === **/
#include <editeng/unofored.hxx>
#include <editeng/editview.hxx>
#include <editeng/unoipset.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdobj.hxx>
#include <editeng/unoprnms.hxx>
#include <svx/globaldrawitempool.hxx>

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
        const SvxItemPropertySet* getTextEnginePropertySet()
        {
            // Propertymap fuer einen Outliner Text
            static const SfxItemPropertyMapEntry aTextEnginePropertyMap[] =
            {
                SVX_UNOEDIT_CHAR_PROPERTIES,
                SVX_UNOEDIT_FONT_PROPERTIES,
                SVX_UNOEDIT_PARA_PROPERTIES,
                { MAP_CHAR_LEN("TextUserDefinedAttributes"), EE_CHAR_XMLATTRIBS, &::getCppuType( static_cast< const Reference< XNameContainer >* >( NULL ) ), 0, 0 },
                { MAP_CHAR_LEN("ParaUserDefinedAttributes"), EE_PARA_XMLATTRIBS, &::getCppuType( static_cast< const Reference< XNameContainer >* >( NULL ) ), 0, 0 },
                { NULL, 0, 0, NULL, 0, 0 }
            };
            static SvxItemPropertySet aTextEnginePropertySet( aTextEnginePropertyMap, GetGlobalDrawObjectItemPool() );
            return &aTextEnginePropertySet;
        }
    }

    //====================================================================
    //= ORichTextUnoWrapper
    //====================================================================
    //--------------------------------------------------------------------
    ORichTextUnoWrapper::ORichTextUnoWrapper( EditEngine& _rEngine, IEngineTextChangeListener* _pTextChangeListener )
        :SvxUnoText( getTextEnginePropertySet() )
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
        sal_uInt16 viewCount = m_rEngine.GetViewCount();
        for ( sal_uInt16 view = 0; view < viewCount; ++view )
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

