/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: htmlvsh.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:46:22 $
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

#ifndef _HTMLVSH_HXX
#define _HTMLVSH_HXX

#ifndef _CALBCK_HXX
#include <calbck.hxx>
#endif
class ViewShell;


class SwHTMLViewShellClient : public SwClient
{
    virtual void Modify( SfxPoolItem *pOld, SfxPoolItem *pNew );

public:

    SwHTMLViewShellClient( ViewShell *pVSh );

    virtual ~SwHTMLViewShellClient();

    void Register( ViewShell *pVsh );
    void DeRegister();

    /*inline*/ ViewShell *GetViewShell(); // im swhtml.cxx
};



#endif


