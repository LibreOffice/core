/*************************************************************************
 *
 *  $RCSfile: config.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:57:24 $
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
#ifndef CONFIG_HXX
#define CONFIG_HXX

#ifndef _SFXBRDCST_HXX //autogen
#include <svtools/brdcst.hxx>
#endif
#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif
#ifndef _SFXCFGITEM_HXX //autogen
#include <sfx2/cfgitem.hxx>
#endif

#ifndef _FORMAT_HXX
#include "format.hxx"
#endif

#define HINT_CONFIGCHANGED      10002
#define SMCONFIGITEM            (SID_SMA_START+1)


class SmPreferenceDialog;
class SmPrintDialog;
class SmPrintOptionDialog;
class SfxItemSet;

enum SmPrintSize { PRINT_SIZE_NORMAL, PRINT_SIZE_SCALED, PRINT_SIZE_ZOOMED };

class SmConfig : public SfxBroadcaster, public SfxListener, public SfxConfigItem
{
    String          aSymbolFile;
    SmFormat        aStandardFormat;
    SmFontPickList  vFontPickList[7];

    SmPrintSize     ePrintSize;
    USHORT          nPrintZoom;

    BOOL            bPrintTitle,
                    bPrintText,
                    bPrintFrame,
                    bWarnNoSymbols,
                    bToolBoxVisible,
                    bCmdBoxWindow,
                    bAutoRedraw,
                    bFormulaCursor,
                    bNoRightSpaces;     // ignorieren von ~ und ` am Zeilenende

    virtual void SFX_NOTIFY(SfxBroadcaster &rBC, const TypeId &rBCType,
                            const SfxHint &rHint, const TypeId &rHintType);

    void        ConfigChangedAction();
    void        SetValueIfNE(BOOL &rItem, const BOOL bVal);

public:
    SmConfig();
    virtual ~SmConfig();

    virtual int     Load (SvStream &rStream);
    virtual BOOL    Store(SvStream &rStream);

    virtual void    UseDefault();
    virtual String  GetName() const;

    SmFormat &       GetFormat() { return aStandardFormat; }
    SmFontPickList & GetFontPickList(USHORT nIdent) { return vFontPickList[nIdent]; }

    const String &  GetSymbolFile() const { return aSymbolFile; }
    void            SetSymbolFile(const String &rText);

    SmPrintSize     GetPrintSize() const { return ePrintSize; }
    USHORT          GetPrintZoom() const { return nPrintZoom; }

    BOOL IsCmdBoxWindowEnabled() const { return bCmdBoxWindow; }

    BOOL IsPrintTitle() const    { return bPrintTitle; }
    BOOL IsPrintText()  const    { return bPrintText; }
    BOOL IsPrintFrame() const    { return bPrintFrame; }
    BOOL IsNoRightSpaces() const { return bNoRightSpaces; }

    BOOL IsToolBoxVisible() const     { return bToolBoxVisible; }
    void SetToolBoxVisible(BOOL bVal) { SetValueIfNE(bToolBoxVisible, bVal); }

    BOOL IsAutoRedraw() const         { return bAutoRedraw; }
    void SetAutoRedraw(BOOL bVal)     { SetValueIfNE(bAutoRedraw, bVal); }

    BOOL IsWarnNoSymbols() const      { return bWarnNoSymbols; }
    void SetWarnNoSymbols(BOOL bVal)  { SetValueIfNE(bWarnNoSymbols, bVal); }

    BOOL IsShowFormulaCursor() const      { return bFormulaCursor; }
    void SetShowFormulaCursor(BOOL bVal)  { SetValueIfNE(bFormulaCursor, bVal); }

    void ItemSetToConfig(const SfxItemSet &rSet);
    void ConfigToItemSet(SfxItemSet &rSet) const;

    friend SvStream & operator << (SvStream &rStream, const SmConfig &rConfig);
    friend SvStream & operator >> (SvStream &rStream, SmConfig &rConfig);
};

#endif


