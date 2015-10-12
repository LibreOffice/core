/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/*************************************************************************
* @file
* Breaks override of Wordpro.
************************************************************************/
#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPBREAKSOVERRIDE_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPBREAKSOVERRIDE_HXX

#include "lwpoverride.hxx"

class LwpObjectStream;
class LwpAtomHolder;
class LwpBreaksOverride : public LwpOverride
{

public:
    LwpBreaksOverride();
    virtual ~LwpBreaksOverride();

    virtual LwpBreaksOverride* clone() const override;

    enum
    {
        BO_PAGEBEFORE   = 0x01, // page break before this style
        BO_PAGEAFTER    = 0x02, // page break after this style
        BO_KEEPTOGETHER = 0x04,
        BO_COLBEFORE    = 0x08, // col break before this style
        BO_COLAFTER     = 0x10, // col break after this style
        BO_KEEPPREV     = 0x20, // not with-PAGE BEF, COL BEF or WITHIN
        BO_KEEPNEXT     = 0x40, // not with-PAGE AFT, COL AFT or WITHIN
        BO_USENEXTSTYLE = 0x80, // use next style name
        BO_NEXTSTYLE    = 0x100 // next style name
    };
public:
    virtual void Read(LwpObjectStream *pStrm) override;

    void Override(LwpBreaksOverride* pOther);

    inline bool IsPageBreakBefore();
    inline bool IsPageBreakAfter();
    inline bool IsPageBreakWithin();
    inline bool IsColumnBreakBefore();
    inline bool IsColumnBreakAfter();
    inline bool IsKeepWithNext();
    inline bool IsKeepWithPrevious();
    inline bool IsUseNextStyle();

    inline bool IsPageBreakBeforeOverridden();
    inline bool IsPageBreakAfterOverridden();
    inline bool IsPageBreakWithinOverridden();
    inline bool IsColumnBreakBeforeOverridden();
    inline bool IsColumnBreakAfterOverridden();
    inline bool IsKeepWithNextOverridden();
    inline bool IsKeepWithPreviousOverridden();
    inline bool IsUseNextStyleOverridden();

    inline void RevertPageBreakBefore();
    inline void RevertPageBreakAfter();
    inline void RevertPageBreakWithin();
    inline void RevertColumnBreakBefore();
    inline void RevertColumnBreakAfter();
    inline void RevertKeepWithNext();
    inline void RevertKeepWithPrevious();
    inline void RevertUseNextStyle();

    void OverridePageBreakBefore(bool bVal);
    void OverridePageBreakAfter(bool bVal);
    void OverridePageBreakWithin(bool bVal);
    void OverrideColumnBreakBefore(bool bVal);
    void OverrideColumnBreakAfter(bool bVal);
    void OverrideKeepWithNext(bool bVal);
    void OverrideKeepWithPrevious(bool bVal);
    void OverrideUseNextStyle(bool bVal);

protected:
    LwpBreaksOverride(LwpBreaksOverride const& rOther);

private:
    LwpBreaksOverride& operator=(const LwpBreaksOverride& rOther) = delete;

private:
    LwpAtomHolder       *m_pNextStyle;
};

inline bool LwpBreaksOverride::IsPageBreakBefore()
{
    return ((m_nValues & BO_PAGEBEFORE) != 0);
}

inline bool LwpBreaksOverride::IsPageBreakAfter()
{
    return ((m_nValues & BO_PAGEAFTER) != 0);
}
inline bool LwpBreaksOverride::IsPageBreakWithin()
{
    return ((m_nValues & BO_KEEPTOGETHER) != 0);
}
inline bool LwpBreaksOverride::IsColumnBreakBefore()
{
    return ((m_nValues & BO_COLBEFORE) != 0);
}
inline bool LwpBreaksOverride::IsColumnBreakAfter()
{
    return ((m_nValues & BO_COLAFTER) != 0);
}
inline bool LwpBreaksOverride::IsKeepWithNext()
{
    return ((m_nValues & BO_KEEPNEXT) != 0);
}
inline bool LwpBreaksOverride::IsKeepWithPrevious()
{
    return ((m_nValues & BO_KEEPPREV) != 0);
}
inline bool LwpBreaksOverride::IsUseNextStyle()
{
    return ((m_nValues & BO_USENEXTSTYLE) != 0);
}

inline bool LwpBreaksOverride::IsPageBreakBeforeOverridden()
{
    return ((m_nOverride& BO_PAGEBEFORE) != 0);
}
inline bool LwpBreaksOverride::IsPageBreakAfterOverridden()
{
    return ((m_nOverride& BO_PAGEAFTER) != 0);
}
inline bool LwpBreaksOverride::IsPageBreakWithinOverridden()
{
    return ((m_nOverride& BO_KEEPTOGETHER) != 0);
}
inline bool LwpBreaksOverride::IsColumnBreakBeforeOverridden()
{
    return ((m_nOverride& BO_COLBEFORE) != 0);
}
inline bool LwpBreaksOverride::IsColumnBreakAfterOverridden()
{
    return ((m_nOverride& BO_COLAFTER) != 0);
}
inline bool LwpBreaksOverride::IsKeepWithNextOverridden()
{
    return ((m_nOverride& BO_KEEPNEXT) != 0);
}
inline bool LwpBreaksOverride::IsKeepWithPreviousOverridden()
{
    return ((m_nOverride& BO_KEEPPREV) != 0);
}
inline bool LwpBreaksOverride::IsUseNextStyleOverridden()
{
    return ((m_nOverride& BO_USENEXTSTYLE) != 0);
}

inline void LwpBreaksOverride::RevertPageBreakBefore()
{
    LwpOverride::Override(BO_PAGEBEFORE, STATE_STYLE);
}
inline void LwpBreaksOverride::RevertPageBreakAfter()
{
    LwpOverride::Override(BO_PAGEAFTER, STATE_STYLE);
}
inline void LwpBreaksOverride::RevertPageBreakWithin()
{
    LwpOverride::Override(BO_KEEPTOGETHER, STATE_STYLE);
}
inline void LwpBreaksOverride::RevertColumnBreakBefore()
{
    LwpOverride::Override(BO_COLBEFORE, STATE_STYLE);
}
inline void LwpBreaksOverride::RevertColumnBreakAfter()
{
    LwpOverride::Override(BO_COLAFTER, STATE_STYLE);
}
inline void LwpBreaksOverride::RevertKeepWithNext()
{
    LwpOverride::Override(BO_KEEPNEXT, STATE_STYLE);
}
inline void LwpBreaksOverride::RevertKeepWithPrevious()
{
    LwpOverride::Override(BO_KEEPPREV, STATE_STYLE);
}
inline void LwpBreaksOverride::RevertUseNextStyle()
{
    LwpOverride::Override(BO_USENEXTSTYLE, STATE_STYLE);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
