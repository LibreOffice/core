/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: editimplementation.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:16:55 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
void GenericEditImplementation< EDIT >::SetReadOnly( BOOL bReadOnly )
{
    m_rEdit.SetReadOnly( bReadOnly );
}

//----------------------------------------------------------------------
template <class EDIT>
BOOL GenericEditImplementation< EDIT >::IsReadOnly() const
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

