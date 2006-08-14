/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: extinput.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2006-08-14 15:21:51 $
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
#ifndef _EXTINPUT_HXX
#define _EXTINPUT_HXX

#ifndef _SVSTDARR_HXX
#define _SVSTDARR_USHORTS
#include <svtools/svstdarr.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef INCLUDED_I18NPOOL_LANG_H
#include <i18npool/lang.h>
#endif

class CommandExtTextInputData;

class SwExtTextInput : public SwPaM
{
    SvUShorts aAttrs;
    String sOverwriteText;
    BOOL bInsText : 1;
    BOOL bIsOverwriteCursor : 1;
    LanguageType eInputLanguage;
public:
    SwExtTextInput( const SwPaM& rPam, Ring* pRing = 0 );
    virtual ~SwExtTextInput();

    void SetInputData( const CommandExtTextInputData& rData );
    const SvUShorts& GetAttrs() const   { return aAttrs; }
    void SetInsText( BOOL bFlag )       { bInsText = bFlag; }
    BOOL IsOverwriteCursor() const      { return bIsOverwriteCursor; }
    void SetOverwriteCursor( BOOL bFlag );
    void SetLanguage(LanguageType eSet) { eInputLanguage = eSet;}
};

#endif  //_EXTINPUT_HXX

