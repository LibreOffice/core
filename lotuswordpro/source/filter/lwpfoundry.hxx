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
 Jan 2005           Created
 ************************************************************************/

#ifndef _LWPFOUNDRY_HXX
#define _LWPFOUNDRY_HXX

#include "lwpheader.hxx"

#ifndef _LWPOBJECTID_HXX
#include "lwpobjid.hxx"
#endif

#ifndef _LWPOBJECTSTREAM_HXX
#include "lwpobjstrm.hxx"
#endif

#include "lwpcolor.hxx"
#include "lwpbasetype.hxx"
#include "lwpfont.hxx"
#include "lwpdropcapmgr.hxx"
class LwpDocument;
class LwpBookMark;

#include <vector>
#define TAG_USER_VERSION    0x72655655UL        // "UVer"


class LwpVersionManager
{
public:
    LwpVersionManager(){}
    ~LwpVersionManager(){}
public:
    void Read(LwpObjectStream *pStrm);
    void Skip(LwpObjectStream *pStrm);
};

class LwpObjectManager
{
public:
    LwpObjectManager(){}
    ~LwpObjectManager(){}
private:
    LwpObjectID m_Division;
public:
    void Read(LwpObjectStream *pStrm);
};

class LwpNumberManager
{
public:
    LwpNumberManager(){}
    ~LwpNumberManager(){}
private:
    LwpObjectID m_TableRange;
public:
    void Read(LwpObjectStream *pStrm);
    LwpObjectID GetTableRangeID(){ return m_TableRange;}
};

class LwpBulletManager
{
public:
    LwpBulletManager(){}
    ~LwpBulletManager(){}
private:
    LwpObjectID m_Head;
public:
    void Read(LwpObjectStream *pStrm);
    LwpObjectID* GetHeadID() { return &m_Head;}
};

class LwpContent;
class LwpContentManager
{
public:
    LwpContentManager(){}
    ~LwpContentManager(){}
private:
    LwpObjectID m_ContentList;
    LwpObjectID m_EnumHead;
    LwpObjectID m_EnumTail;
    LwpObjectID m_OleObjCount;

    LwpObjectID m_GrapHead;
    LwpObjectID m_GrapTail;
    LwpObjectID m_OleHead;
    LwpObjectID m_OleTail;
public:
    inline LwpObjectID* GetContentList() { return &m_ContentList; }
    inline LwpObjectID* GetGraphicListHead() { return &m_GrapHead; }
    LwpContent* EnumContents(LwpContent* pContent);

public:
    void Read(LwpObjectStream *pStrm);
};

class LwpPieceManager
{
public:
    LwpPieceManager(){}
    ~LwpPieceManager(){}
private:
    LwpObjectID m_GeometryPieceList;
    LwpObjectID m_ScalePieceList;
    LwpObjectID m_MarginsPieceList;
    LwpObjectID m_ColumnsPieceList;
    LwpObjectID m_BorderStuffPieceList;
    LwpObjectID m_GutterStuffPieceList;
    LwpObjectID m_BackgroundStuffPieceList;
    LwpObjectID m_JoinStuffPieceList;
    LwpObjectID m_ShadowPieceList;
    LwpObjectID m_NumericsPieceList;
    LwpObjectID m_RelativityPieceList;
    LwpObjectID m_AlignmentPieceList;
    LwpObjectID m_IndentPieceList;
    LwpObjectID m_ParaBorderPieceList;
    LwpObjectID m_SpacingPieceList;
    LwpObjectID m_BreaksPieceList;
    LwpObjectID m_NumberingPieceList;
    LwpObjectID m_TabPieceList;
    LwpObjectID m_CharacterBorderPieceList;
    LwpObjectID m_AmikakePieceList;

    LwpObjectID m_ParaBackgroundPieceList;
    LwpObjectID m_ExternalBorderStuffPieceList;
    LwpObjectID m_ExternalJoinStuffPieceList;

public:
    void Read(LwpObjectStream *pStrm);
};

class LwpOrderedObject;
class LwpListList;
class LwpOrderedObjectManager
{
public:
    LwpOrderedObjectManager(){}
    ~LwpOrderedObjectManager(){}
protected:
    LwpObjectID m_Head;
public:
    void Read(LwpObjectStream *pStrm);
    LwpObjectID* GetHeadID() { return &m_Head;}
    LwpOrderedObject* Enumerate(LwpOrderedObject* pLast);
protected:
    LwpListList* GetNextActiveListList(LwpListList* pLast);
};

class LwpStyleManager;
class LwpSection;
class LwpBulletStyleMgr;
class LwpFoundry
{
public:
    LwpFoundry( LwpObjectStream *pStrm, LwpDocument* pDoc );
    ~LwpFoundry();
    void Read(LwpObjectStream *pStrm);
    void RegisterAllLayouts();
private:
    LwpDocument* m_pDoc;
    bool m_bRegisteredAll;
private: //file members
    LwpVersionManager m_VerMgr;
    LwpObjectManager m_ObjMgr;
    LwpObjectID m_MarkerHead;
    LwpObjectID m_FootnoteMgr;
    LwpNumberManager m_NumMgr;
    LwpBulletManager m_BulMgr;
    LwpOrderedObjectManager m_SectionList;

    LwpObjectID m_Layout;   //The head layout
    LwpObjectID m_TextStyle;
    LwpObjectID m_DefaultTextStyle;
    LwpObjectID m_DefaultClickStyle;
    LwpObjectID m_PageStyle;
    LwpObjectID m_FrameStyle;
    LwpObjectID m_TableStyle;
    LwpObjectID m_CellStyle;
    LwpObjectID m_DftFrameStyle;
    LwpObjectID m_DftPageStyle;
    LwpObjectID m_DftTableStyle;
    LwpObjectID m_DftCellStyle;
    LwpObjectID m_DftColumnStyle;
    LwpObjectID m_DftLeftColumnStyle;
    LwpObjectID m_DftRighColumnStyle;

    LwpObjectID m_BookMarkHead;
    LwpObjectID m_DdeLinkHead;
    LwpObjectID m_DirtBagHead;
    LwpObjectID m_NamedOutlineSeqHead;
    LwpObjectID m_EnumLayoutHead;
    LwpObjectID m_EnumLayoutTail;

    LwpObjectID m_NamedObjects;
    sal_uInt32 m_nLastClickHere;

    LwpObjectID m_SmartTextMgr;

    LwpContentManager m_ContentMgr;
    LwpFontManager m_FontMgr;
    LwpPieceManager* m_pPieceMgr;

    LwpObjectID m_DftDropCapStyle;
    LwpObjectID m_DftHeaderStyle;

    LwpObjectID m_DftFooterStyle;
private:
    void ReadStyles(LwpObjectStream *pStrm);
public:
    inline LwpContentManager* GetContentManager() { return &m_ContentMgr; }
    inline LwpObjectID* GetGraphicListHead() { return m_ContentMgr.GetGraphicListHead(); }
    inline LwpFontManager* GetFontManger() { return &m_FontMgr;}
    inline LwpObjectID* GetTextStyleHead()  { return &m_TextStyle;}
    inline LwpObjectID* GetLayout() {return &m_Layout;}
    inline LwpObjectID* GetBulletManagerID() { return m_BulMgr.GetHeadID();}
    inline LwpDocument* GetDocument(){ return m_pDoc;}
    inline LwpNumberManager* GetNumberManager() { return &m_NumMgr;}
    LwpObjectID * GetDefaultTextStyle() ;
private:
    LwpStyleManager* m_pStyleMgr;
    LwpDropcapMgr* m_pDropcapMgr;
    LwpBulletStyleMgr* m_pBulletStyleMgr;
public:
    inline LwpStyleManager* GetStyleManager() { return m_pStyleMgr;}
    LwpBookMark* GetBookMark(LwpObjectID objMarker);
    LwpDropcapMgr* GetDropcapMgr(){return m_pDropcapMgr;}
    LwpContent* EnumContents(LwpContent* pContent);
    LwpSection* EnumSections(LwpSection* pSection);
    LwpBulletStyleMgr* GetBulletStyleMgr(){return m_pBulletStyleMgr;}

    LwpObjectID* FindParaStyleByName(OUString name);
    OUString FindActuralStyleName(OUString name);
};

class LwpStyleManager
{
public:
    LwpStyleManager();
    ~LwpStyleManager();
private:
    LwpFoundry* m_pFoundry;
    struct hashFunc
    {
            size_t operator()( const LwpObjectID& rName ) const
            {
                return rName.HashCode();
            }
    };
    struct eqFunc
    {
            bool operator()( const LwpObjectID& rKey1, const LwpObjectID& rKey2 ) const
            {
                return(rKey1==rKey2);
            }
    };

    typedef boost::unordered_map<LwpObjectID, IXFStyle*, hashFunc, eqFunc> LwpStyleMap;
    LwpStyleMap m_StyleList;
public:
    void SetFoundry(LwpFoundry* pFoundry){m_pFoundry = pFoundry;}
    IXFStyle* AddStyle(LwpObjectID styleObjID, IXFStyle* pStyle);
    IXFStyle* GetStyle(const LwpObjectID &styleObjID);
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
