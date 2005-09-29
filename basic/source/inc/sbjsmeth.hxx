/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sbjsmeth.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2005-09-29 16:31:39 $
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


#ifndef _SB_SBJSMETH_HXX
#define _SB_SBJSMETH_HXX

#ifndef _SB_SBMETH_HXX
#include <sbmeth.hxx>
#endif

// Basic-Modul fuer JavaScript-Sourcen.
// Alle Basic-spezifischen Methoden muessen virtuell ueberladen und deaktiviert
// werden. Die Unterscheidung von normalen Modulen erfolgt uebr RTTI.

class SbJScriptMethod : public SbMethod
{
public:
    SbJScriptMethod( const String&, SbxDataType, SbModule* );
    virtual ~SbJScriptMethod();

    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_JSCRIPTMETH,2);
    TYPEINFO();
};

#ifndef __SB_SBJSCRIPTMETHODREF_HXX
#define __SB_SBJSCRIPTMETHODREF_HXX
SV_DECL_IMPL_REF(SbJScriptMethod)
#endif

#endif
