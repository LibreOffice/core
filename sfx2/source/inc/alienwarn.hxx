/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: alienwarn.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:34:46 $
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
#ifndef _SFX_ALIENWARN_HXX
#define _SFX_ALIENWARN_HXX

#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif

#include <sfx2/basedlgs.hxx>

class SfxAlienWarningDialog : public SfxModalDialog
{
private:
    FixedImage              m_aQueryImage;
    FixedText               m_aInfoText;
    OKButton                m_aKeepCurrentBtn;
    CancelButton            m_aSaveODFBtn;
    HelpButton              m_aMoreInfoBtn;
    FixedLine               m_aOptionLine;
    CheckBox                m_aWarningOnBox;

    void                    InitSize();

public:
             SfxAlienWarningDialog( Window* pParent, const String& _rFormatName );
    virtual ~SfxAlienWarningDialog();
};

#endif // #ifndef _SFX_ALIENWARN_HXX

