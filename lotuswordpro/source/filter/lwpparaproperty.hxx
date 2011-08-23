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
 *  For LWP filter architecture prototype
 ************************************************************************/
/*************************************************************************
 * Change History
 Jan 2005			Created
 ************************************************************************/
#ifndef _LWPPARAPROPERTY_HXX_
#define _LWPPARAPROPERTY_HXX_

#include "lwpobj.hxx"
#include "lwpobjstrm.hxx"
#include "lwpdllist.hxx"

#include "lwpoverride.hxx"
#include "lwppiece.hxx"

/* paragraph property ID's */
#define	PP_OUTLINE_SHOW			0x53484f4cUL	/* "SHOW" */
#define	PP_OUTLINE_HIDE			0x48494445UL	/* "HIDE" */
#define PP_LOCAL_ALIGN			0x414c494eUL	/* "ALIN" */
#define PP_LOCAL_INDENT			0x494e444eUL	/* "INDN" */
#define PP_LOCAL_SPACING		0x5350434eUL	/* "SPCN" */
#define PP_LOCAL_TABRACK		0x54414253UL	/* "TABS" */
#define PP_LOCAL_BREAKS			0x42524b53UL	/* "BRKS" */
#define PP_LOCAL_BULLET			0x42554c4cUL	/* "BULL" */
#define PP_LOCAL_BORDER			0x424f5244UL	/* "BORD" */
#define PP_LOCAL_BACKGROUND		0x4241434bUL	/* "BACK" */
#define PP_LOCAL_NUMBERING		0x4e4d4252UL	/* "NMBR" */
#define PP_LOCAL_KINSOKU		0x4b494e53UL	/* "KINS" */
#define PP_PROPLIST				0x50524f50UL	/* "PROP" */

class LwpParaProperty : public LwpDLList
{
public:
    LwpParaProperty(){};
    virtual ~LwpParaProperty(void){};
    virtual sal_uInt32	GetType(void) = 0;
    inline  LwpParaProperty* GetNext(void);
    inline  LwpParaProperty* GetPrevious(void);

    static LwpParaProperty* ReadPropertyList(LwpObjectStream* pFile,LwpObject* Whole);

};

inline LwpParaProperty* LwpParaProperty::GetNext(void)
{
    return static_cast<LwpParaProperty*>(LwpDLList::GetNext());
}

inline LwpParaProperty* LwpParaProperty::GetPrevious(void)
{
    return static_cast<LwpParaProperty*>(LwpDLList::GetPrevious());
}

//align/indent/spacing
//TO DO:border/backgroud etc


class LwpParaAlignProperty : public LwpParaProperty
{
public:
//		LwpParaAlignProperty(LwpParaAlignProperty* pOther);
        LwpParaAlignProperty(LwpObjectStream* pFile);
        virtual ~LwpParaAlignProperty(void);
        LwpAlignmentOverride* GetAlignment(void);
        sal_uInt32	GetType(void);

private:
//		LwpAlignmentPiece m_Alignment;
        LwpAlignmentOverride* m_pAlignment;
};

inline LwpAlignmentOverride* LwpParaAlignProperty::GetAlignment(void)
{
//	return static_cast<LwpAlignmentOverride*>(m_Alignment.m_pOverride);
    return m_pAlignment; //add by  1-24
}

class LwpParaIndentProperty : public LwpParaProperty
{
public:
        LwpParaIndentProperty(LwpObjectStream* pFile);
        virtual ~LwpParaIndentProperty(void);
        LwpIndentOverride* GetIndent(void);
        sal_uInt32 GetType(void);
        //add by , 03/22/2005
        inline LwpObjectID GetIndentID();
        //end add

private:
        //add by , 03/22/2005
        LwpObjectID m_aIndentID;
        //end add
        LwpIndentOverride* m_pIndent;
};
inline LwpObjectID LwpParaIndentProperty::GetIndentID()
{
    return m_aIndentID;
}
inline LwpIndentOverride* LwpParaIndentProperty::GetIndent(void)
{
    return m_pIndent;
}

class LwpParaSpacingProperty : public LwpParaProperty
{
public:
        LwpParaSpacingProperty(LwpObjectStream* pFile);
        virtual ~LwpParaSpacingProperty(void);
        LwpSpacingOverride* GetSpacing(void);
        sal_uInt32 GetType(void);
private:
        LwpSpacingOverride* m_pSpacing;

};

inline LwpSpacingOverride* LwpParaSpacingProperty::GetSpacing(void)
{
    return m_pSpacing;
}

//add by , 01/25/2005////////////////////////////////////////////
class LwpParaBorderOverride;
class LwpParaBorderProperty : public LwpParaProperty
{
public:
    LwpParaBorderProperty(LwpObjectStream* pStrm);

    inline sal_uInt32 GetType();

    inline LwpParaBorderOverride* GetLocalParaBorder();

private:
    LwpParaBorderOverride* m_pParaBorderOverride;
};

inline LwpParaBorderOverride* LwpParaBorderProperty::GetLocalParaBorder()
{
    return m_pParaBorderOverride;
}

inline sal_uInt32 LwpParaBorderProperty::GetType()
{
    return PP_LOCAL_BORDER;
}

class LwpParaBreaksProperty : public LwpParaProperty
{
public:
    LwpParaBreaksProperty(LwpObjectStream* pStrm);

    inline sal_uInt32 GetType();

    inline LwpBreaksOverride* GetLocalParaBreaks();

private:
    LwpBreaksOverride* m_pBreaks;

};

inline sal_uInt32 LwpParaBreaksProperty::GetType()
{
    return PP_LOCAL_BREAKS;
}
inline LwpBreaksOverride* LwpParaBreaksProperty::GetLocalParaBreaks()
{
    return m_pBreaks;
}


class LwpParaBulletProperty : public LwpParaProperty
{
public:
    LwpParaBulletProperty(LwpObjectStream* pStrm);

    virtual ~LwpParaBulletProperty();

    inline sal_uInt32 GetType();

    inline LwpBulletOverride* GetLocalParaBullet();

private:
    LwpBulletOverride* m_pBullet;
};

inline sal_uInt32 LwpParaBulletProperty::GetType()
{
    return PP_LOCAL_BULLET;
}
inline LwpBulletOverride* LwpParaBulletProperty::GetLocalParaBullet()
{
    return m_pBullet;
}


class LwpParaNumberingProperty : public LwpParaProperty
{
public:
    LwpParaNumberingProperty(LwpObjectStream* pStrm);

    inline sal_uInt32 GetType();

    inline LwpNumberingOverride* GetLocalNumbering() const;
private:
    LwpNumberingOverride* m_pNumberingOverride;
};
inline sal_uInt32 LwpParaNumberingProperty::GetType()
{
    return PP_LOCAL_NUMBERING;
}
inline LwpNumberingOverride* LwpParaNumberingProperty::GetLocalNumbering() const
{
    return m_pNumberingOverride;
}
//end add/////////////////////////////////////////////////////////////


class LwpParaTabRackProperty : public LwpParaProperty
{
public:
    LwpParaTabRackProperty(LwpObjectStream* pStrm);
    ~LwpParaTabRackProperty(void);
    inline sal_uInt32 GetType();

    inline LwpTabOverride* GetTab(); //should add by , note by  1/28

private:
    LwpTabOverride* m_pTabOverride;
};

inline LwpTabOverride* LwpParaTabRackProperty::GetTab()
{
    return m_pTabOverride;
}

inline sal_uInt32 LwpParaTabRackProperty::GetType(void)
{
    return PP_LOCAL_TABRACK;
}


class LwpParaBackGroundProperty : public LwpParaProperty
{
public:
    LwpParaBackGroundProperty(LwpObjectStream* pFile);
    virtual ~LwpParaBackGroundProperty(void);
    LwpBackgroundOverride* GetBackground(void);
    sal_uInt32 GetType(void);
private:
    LwpBackgroundOverride* m_pBackground;
};

inline LwpBackgroundOverride* LwpParaBackGroundProperty::GetBackground(void)
{
    return m_pBackground;
}

inline sal_uInt32 LwpParaBackGroundProperty::GetType()
{
    return PP_LOCAL_BACKGROUND;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
