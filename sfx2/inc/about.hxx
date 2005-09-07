/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: about.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:43:11 $
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
#ifndef _ABOUT_HXX
#define _ABOUT_HXX

// include ---------------------------------------------------------------

#ifndef _RESARY_HXX //autogen
#include <tools/resary.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _ACCEL_HXX //autogen
#include <vcl/accel.hxx>
#endif
#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
#ifndef _STDCTRL_HXX //autogen
#include <svtools/stdctrl.hxx>
#endif
#include "basedlgs.hxx"     // SfxModalDialog

DECLARE_LIST( AccelList, Accelerator* )

// class AboutDialog -----------------------------------------------------

class AboutDialog : public SfxModalDialog
{
private:
    OKButton        aOKButton;
    Image           aAppLogo;

    FixedInfo       aVersionText;
    FixedInfo       aCopyrightText;
    FixedInfo       aBuildData;

    ResStringArray  aDeveloperAry;
    String          aDevVersionStr;
    String          aAccelStr;
    String          aVersionData;

    AccelList       aAccelList;

    AutoTimer       aTimer;
    long            nOff;
    long            nEnd;

    BOOL            bNormal;

protected:
    virtual BOOL    Close();
    virtual void    Paint( const Rectangle& );

public:
    AboutDialog( Window* pParent, const ResId& rId, const String& rVerStr );
    ~AboutDialog();

    DECL_LINK( TimerHdl, Timer * );
    DECL_LINK( AccelSelectHdl, Accelerator * );
};

#endif // #ifndef _ABOUT_HXX


