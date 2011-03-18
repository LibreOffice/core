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
String GenericEditImplementation< EDIT >::GetText( LineEnd ) const
{
    // ignore the line end - this base implementation does not support it
    return m_rEdit.GetText( );
}

//----------------------------------------------------------------------
template <class EDIT>
void GenericEditImplementation< EDIT >::SetText( const String& _rStr )
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
void GenericEditImplementation< EDIT >::ReplaceSelected( const String& _rStr )
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
String GenericEditImplementation< EDIT >::GetSelected( LineEnd ) const
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
