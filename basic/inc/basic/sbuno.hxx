/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sbuno.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:51:27 $
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

#ifndef _SB_SBUNO_HXX
#define _SB_SBUNO_HXX

#ifndef _SBX_SBXOBJECT_HXX //autogen
#include <basic/sbxobj.hxx>
#endif

namespace com { namespace sun { namespace star { namespace uno { class Any; }}}}

// Liefert ein SbxObject, das ein Uno-Interface wrappt
// Implementiert in basic/source/classes/sbunoobj.cxx
SbxObjectRef GetSbUnoObject( const String& aName, const com::sun::star::uno::Any& aUnoObj_ );

// Force creation of all properties for debugging
void createAllObjectProperties( SbxObject* pObj );

::com::sun::star::uno::Any sbxToUnoValue( SbxVariable* pVar );

#endif

