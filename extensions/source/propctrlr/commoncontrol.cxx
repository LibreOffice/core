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

#include "commoncontrol.hxx"
#include "pcrcommon.hxx"
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/combobox.hxx>
#include <toolkit/helper/vclunohelper.hxx>


namespace pcr
{


    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::inspection::XPropertyControlContext;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::inspection::XPropertyControl;

    CommonBehaviourControlHelper::CommonBehaviourControlHelper( sal_Int16 _nControlType, XPropertyControl& _rAntiImpl )
        :m_nControlType( _nControlType )
        ,m_rAntiImpl( _rAntiImpl )
        ,m_bModified( false )
    {
    }


    CommonBehaviourControlHelper::~CommonBehaviourControlHelper()
    {
    }

    void SAL_CALL CommonBehaviourControlHelper::setControlContext( const Reference< XPropertyControlContext >& _controlcontext ) throw (RuntimeException)
    {
        m_xContext = _controlcontext;
    }

    void SAL_CALL CommonBehaviourControlHelper::notifyModifiedValue(  ) throw (RuntimeException)
    {
        if ( isModified() && m_xContext.is() )
        {
            try
            {
                m_xContext->valueChanged( &m_rAntiImpl );
                m_bModified = false;
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
    }


    void CommonBehaviourControlHelper::autoSizeWindow()
    {
        ScopedVclPtrInstance< ComboBox > aComboBox(getVclWindow(), WB_DROPDOWN);
        aComboBox->SetPosSizePixel(Point(0,0), Size(100,100));
        getVclWindow()->SetSizePixel(aComboBox->GetSizePixel());

        // TODO/UNOize: why do the controls this themselves? Shouldn't this be the task
        // of the browser listbox/line?
    }


    void CommonBehaviourControlHelper::activateNextControl() const
    {
        try
        {
            if ( m_xContext.is() )
                m_xContext->activateNextControl( &m_rAntiImpl );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }


    IMPL_LINK_NOARG_TYPED( CommonBehaviourControlHelper, EditModifiedHdl, Edit&, void )
    {
        setModified();
    }

    IMPL_LINK_NOARG_TYPED( CommonBehaviourControlHelper, ModifiedHdl, ListBox&, void )
    {
        setModified();
    }

    IMPL_LINK_NOARG_TYPED( CommonBehaviourControlHelper, GetFocusHdl, Control&, void )
    {
        try
        {
            if ( m_xContext.is() )
                m_xContext->focusGained( &m_rAntiImpl );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }


    IMPL_LINK_NOARG_TYPED( CommonBehaviourControlHelper, LoseFocusHdl, Control&, void )
    {
        // TODO/UNOize: should this be outside the default control's implementations? If somebody
        // has an own control implementation, which does *not* do this - would this be allowed?
        // If not, then we must move this logic out of here.
        notifyModifiedValue();
    }


} // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
