/*************************************************************************
 *
 *  $RCSfile: opengrf.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 14:59:46 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _SVX_OPENGRF_HXX
#define _SVX_OPENGRF_HXX

#include <memory>       // auto_ptr

#ifndef _FILTER_HXX
#include <svtools/filter.hxx>
#endif


struct  SvxOpenGrf_Impl;

class SvxOpenGraphicDialog
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
    SvxOpenGraphicDialog    (const SvxOpenGraphicDialog&);
    SvxOpenGraphicDialog& operator = ( const SvxOpenGraphicDialog & );

    const std::auto_ptr< SvxOpenGrf_Impl >  mpImpl;
};

#endif // _SVX_OPENGRF_HXX

