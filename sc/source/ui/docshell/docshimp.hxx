/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: docshimp.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-22 10:46:48 $
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
#ifndef SC_DOCSHELLIMP_HXX
#define SC_DOCSHELLIMP_HXX

#include <svtools/ctrltool.hxx>
#include <sfx2/docinsert.hxx>
#include <sfx2/request.hxx>

struct DocShell_Impl
{
    bool                    bIgnoreLostRedliningWarning;
    FontList*               pFontList;
    sfx2::DocumentInserter* pDocInserter;
    SfxRequest*             pRequest;

    DocShell_Impl() :
          bIgnoreLostRedliningWarning( false )
        , pFontList( NULL )
        , pDocInserter( NULL )
        , pRequest( NULL )
    {}

    ~DocShell_Impl()
    {
        delete pFontList;
        delete pDocInserter;
        delete pRequest;
    }
};

#endif

