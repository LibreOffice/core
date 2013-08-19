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

#ifndef SVTOOLS_IN_EDITBROWSEBOX_HXX
#error "not to be included directly!"
#endif

//----------------------------------------------------------------------
template <class EDIT>
GenericEditImplementation< EDIT >::GenericEditImplementation( EDIT& _rEdit )
    :m_rEdit( _rEdit )
{
}

//----------------------------------------------------------------------
template <class EDIT>
Control& GenericEditImplementation< EDIT >::GetControl()
{
    return m_rEdit;
}

//----------------------------------------------------------------------
template <class EDIT>
OUString GenericEditImplementation< EDIT >::GetText( LineEnd ) const
{
    // ignore the line end - this base implementation does not support it
    return m_rEdit.GetText( );
}

//----------------------------------------------------------------------
template <class EDIT>
void GenericEditImplementation< EDIT >::SetText( const OUString& _rStr )
{
    m_rEdit.SetText( _rStr );
}

//----------------------------------------------------------------------
template <class EDIT>
Selection GenericEditImplementation< EDIT >::GetSelection() const
{
    return m_rEdit.GetSelection( );
}

//----------------------------------------------------------------------
template <class EDIT>
void GenericEditImplementation< EDIT >::SetSelection( const Selection& _rSelection )
{
    m_rEdit.SetSelection( _rSelection );
}

//----------------------------------------------------------------------
template <class EDIT>
void GenericEditImplementation< EDIT >::SetReadOnly( sal_Bool bReadOnly )
{
    m_rEdit.SetReadOnly( bReadOnly );
}

//----------------------------------------------------------------------
template <class EDIT>
sal_Bool GenericEditImplementation< EDIT >::IsReadOnly() const
{
    return m_rEdit.IsReadOnly();
}

//----------------------------------------------------------------------
template <class EDIT>
void GenericEditImplementation< EDIT >::ReplaceSelected( const OUString& _rStr )
{
    m_rEdit.ReplaceSelected( _rStr );
}

//----------------------------------------------------------------------
template <class EDIT>
void GenericEditImplementation< EDIT >::DeleteSelected()
{
    m_rEdit.DeleteSelected();
}

//----------------------------------------------------------------------
template <class EDIT>
OUString GenericEditImplementation< EDIT >::GetSelected( LineEnd ) const
{
    return m_rEdit.GetSelected( );
}

//----------------------------------------------------------------------
template <class EDIT>
void GenericEditImplementation< EDIT >::SetMaxTextLen( xub_StrLen _nMaxLen )
{
    m_rEdit.SetMaxTextLen( _nMaxLen );
}

//----------------------------------------------------------------------
template <class EDIT>
xub_StrLen GenericEditImplementation< EDIT >::GetMaxTextLen() const
{
    return (xub_StrLen)m_rEdit.GetMaxTextLen( );
}

//----------------------------------------------------------------------
template <class EDIT>
void GenericEditImplementation< EDIT >::SetModified()
{
    m_rEdit.SetModifyFlag();
}

//----------------------------------------------------------------------
template <class EDIT>
sal_Bool GenericEditImplementation< EDIT >::IsModified() const
{
    return m_rEdit.IsModified();
}

//----------------------------------------------------------------------
template <class EDIT>
void GenericEditImplementation< EDIT >::ClearModified()
{
    m_rEdit.ClearModifyFlag();
}

//----------------------------------------------------------------------
template <class EDIT>
void GenericEditImplementation< EDIT >::SetModifyHdl( const Link& _rLink )
{
    m_rEdit.SetModifyHdl( _rLink );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
