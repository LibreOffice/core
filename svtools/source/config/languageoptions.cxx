/*************************************************************************
 *
 *  $RCSfile: languageoptions.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: os $ $Date: 2002-08-14 11:02:24 $
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

#pragma hdrstop

#ifndef _SVTOOLS_LANGUAGEOPTIONS_HXX
#include "languageoptions.hxx"
#endif
#ifndef _SVTOOLS_CJKOPTIONS_HXX
#include "cjkoptions.hxx"
#endif
#ifndef _SVTOOLS_CTLOPTIONS_HXX
#include "ctloptions.hxx"
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

// global ----------------------------------------------------------------------

static ::osl::Mutex aALMutex;

// class SvtLanguageOptions ----------------------------------------------------

SvtLanguageOptions::SvtLanguageOptions( sal_Bool _bDontLoad )
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( aALMutex );

    m_pCJKOptions = new SvtCJKOptions( _bDontLoad );
    m_pCTLOptions = new SvtCTLOptions( _bDontLoad );
}
//------------------------------------------------------------------------------
SvtLanguageOptions::~SvtLanguageOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( aALMutex );

    delete m_pCJKOptions;
    delete m_pCTLOptions;
}
//------------------------------------------------------------------------------
// CJK options -----------------------------------------------------------------
//------------------------------------------------------------------------------
sal_Bool SvtLanguageOptions::IsCJKFontEnabled() const
{
    return m_pCJKOptions->IsCJKFontEnabled();
}
//------------------------------------------------------------------------------
sal_Bool SvtLanguageOptions::IsVerticalTextEnabled() const
{
    return m_pCJKOptions->IsVerticalTextEnabled();
}
//------------------------------------------------------------------------------
sal_Bool SvtLanguageOptions::IsAsianTypographyEnabled() const
{
    return m_pCJKOptions->IsAsianTypographyEnabled();
}
//------------------------------------------------------------------------------
sal_Bool SvtLanguageOptions::IsJapaneseFindEnabled() const
{
    return m_pCJKOptions->IsJapaneseFindEnabled();
}
//------------------------------------------------------------------------------
sal_Bool SvtLanguageOptions::IsRubyEnabled() const
{
    return m_pCJKOptions->IsRubyEnabled();
}
//------------------------------------------------------------------------------
sal_Bool SvtLanguageOptions::IsChangeCaseMapEnabled() const
{
    return m_pCJKOptions->IsChangeCaseMapEnabled();
}
//------------------------------------------------------------------------------
sal_Bool SvtLanguageOptions::IsDoubleLinesEnabled() const
{
    return m_pCJKOptions->IsDoubleLinesEnabled();
}
//------------------------------------------------------------------------------
sal_Bool SvtLanguageOptions::IsEmphasisMarksEnabled() const
{
    return m_pCJKOptions->IsEmphasisMarksEnabled();
}
//------------------------------------------------------------------------------
sal_Bool SvtLanguageOptions::IsVerticalCallOutEnabled() const
{
    return m_pCJKOptions->IsVerticalCallOutEnabled();
}
//------------------------------------------------------------------------------
void SvtLanguageOptions::SetAll( sal_Bool _bSet )
{
    m_pCJKOptions->SetAll( _bSet );
}
//------------------------------------------------------------------------------
sal_Bool SvtLanguageOptions::IsAnyEnabled() const
{
    return m_pCJKOptions->IsAnyEnabled();
}
//------------------------------------------------------------------------------
// CTL options -----------------------------------------------------------------
//------------------------------------------------------------------------------
void SvtLanguageOptions::SetCTLFontEnabled( sal_Bool _bEnabled )
{
    m_pCTLOptions->SetCTLFontEnabled( _bEnabled );
}
//------------------------------------------------------------------------------
sal_Bool SvtLanguageOptions::IsCTLFontEnabled() const
{
    return m_pCTLOptions->IsCTLFontEnabled();
}
//------------------------------------------------------------------------------
void SvtLanguageOptions::SetCTLSequenceChecking( sal_Bool _bEnabled )
{
    m_pCTLOptions->SetCTLSequenceChecking( _bEnabled );
}
//------------------------------------------------------------------------------
sal_Bool SvtLanguageOptions::IsCTLSequenceChecking() const
{
    return m_pCTLOptions->IsCTLSequenceChecking();
}

