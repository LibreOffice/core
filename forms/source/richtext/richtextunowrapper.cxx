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

#include "richtextunowrapper.hxx"

#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <editeng/unofored.hxx>
#include <editeng/editview.hxx>
#include <editeng/unoipset.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdobj.hxx>
#include <editeng/unoprnms.hxx>


namespace frm
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;


    namespace
    {
        const SvxItemPropertySet* getTextEnginePropertySet()
        {
            // property map for an outliner text
            static const SfxItemPropertyMapEntry aTextEnginePropertyMap[] =
            {
                SVX_UNOEDIT_CHAR_PROPERTIES,
                SVX_UNOEDIT_FONT_PROPERTIES,
                SVX_UNOEDIT_PARA_PROPERTIES,
                { u"TextUserDefinedAttributes", EE_CHAR_XMLATTRIBS, cppu::UnoType<XNameContainer>::get(), 0, 0 },
                { u"ParaUserDefinedAttributes", EE_PARA_XMLATTRIBS, cppu::UnoType<XNameContainer>::get(), 0, 0 },
                { u"", 0, css::uno::Type(), 0, 0 }
            };
            static SvxItemPropertySet aTextEnginePropertySet( aTextEnginePropertyMap, SdrObject::GetGlobalDrawObjectItemPool() );
            return &aTextEnginePropertySet;
        }
    }

    ORichTextUnoWrapper::ORichTextUnoWrapper( EditEngine& _rEngine, IEngineTextChangeListener* _pTextChangeListener )
        :SvxUnoText( getTextEnginePropertySet() )
    {
        SetEditSource( new RichTextEditSource( _rEngine, _pTextChangeListener ) );
    }


    ORichTextUnoWrapper::~ORichTextUnoWrapper() noexcept
    {
    }

    RichTextEditSource::RichTextEditSource( EditEngine& _rEngine, IEngineTextChangeListener* _pTextChangeListener )
        :m_rEngine              ( _rEngine                               )
        ,m_pTextForwarder       ( new SvxEditEngineForwarder( _rEngine ) )
        ,m_pTextChangeListener  ( _pTextChangeListener                   )
    {
    }


    RichTextEditSource::~RichTextEditSource()
    {
    }


    std::unique_ptr<SvxEditSource> RichTextEditSource::Clone() const
    {
        return std::unique_ptr<SvxEditSource>(new RichTextEditSource( m_rEngine, m_pTextChangeListener ));
    }


    SvxTextForwarder* RichTextEditSource::GetTextForwarder()
    {
        return m_pTextForwarder.get();
    }


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


}  // namespace frm


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
