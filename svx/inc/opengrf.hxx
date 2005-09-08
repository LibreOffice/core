/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: opengrf.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 18:10:18 $
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
#ifndef _SVX_OPENGRF_HXX
#define _SVX_OPENGRF_HXX

#include <memory>       // auto_ptr

#ifndef _FILTER_HXX
#include <svtools/filter.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif


struct  SvxOpenGrf_Impl;

class SVX_DLLPUBLIC SvxOpenGraphicDialog
{
public:
    SvxOpenGraphicDialog    ( const String& rTitle );
    ~SvxOpenGraphicDialog   ();

    short                   Execute();

    void                    SetPath( const String& rPath );
    void                    SetPath( const String& rPath, sal_Bool bLinkState );
    String                  GetPath() const;

    int                     GetGraphic(Graphic&) const;

    void                    EnableLink(sal_Bool);
    void                    AsLink(sal_Bool);
    sal_Bool                IsAsLink() const;

    String                  GetCurrentFilter() const;
    void                    SetCurrentFilter(const String&);

    /// Set dialog help id at FileDlgHelper
    void                    SetControlHelpIds( const INT16* _pControlId, const INT32* _pHelpId );
    /// Set control help ids at FileDlgHelper
    void                    SetDialogHelpId( const INT32 _nHelpId );
private:
    // disable copy and assignment
    SVX_DLLPRIVATE SvxOpenGraphicDialog (const SvxOpenGraphicDialog&);
    SVX_DLLPRIVATE SvxOpenGraphicDialog& operator = ( const SvxOpenGraphicDialog & );

    const std::auto_ptr< SvxOpenGrf_Impl >  mpImpl;
};

#endif // _SVX_OPENGRF_HXX

