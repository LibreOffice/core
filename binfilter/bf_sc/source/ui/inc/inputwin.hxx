/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SC_INPUTWIN_HXX
#define SC_INPUTWIN_HXX


#include <vcl/toolbox.hxx>
#include <bf_sfx2/childwin.hxx>
#include <bf_svtools/lstner.hxx>
#include <vcl/combobox.hxx>
#include <vcl/window.hxx>
#include <bf_svtools/transfer.hxx>
namespace binfilter {

class ScEditEngineDefaulter;
class EditView;
struct ESelection;
class ScInputHandler;
class ScAccessibleEditLineTextData;
struct EENotify;
class ScRangeList;

//========================================================================

class ScTextWnd : public Window, public DragSourceHelper		// edit window
{
public:
                    ScTextWnd( Window* pParent );
    virtual			~ScTextWnd();



                        // fuer FunktionsAutopiloten






    DECL_LINK( NotifyHdl, EENotify* );

protected:
    virtual void	Resize();




private:
    void			ImplInitSettings();
    void			UpdateAutoCorrFlag();

private:
    String		aString;
    Font		aTextFont;
    ScEditEngineDefaulter*	pEditEngine;			// erst bei Bedarf angelegt
    EditView*	pEditView;
    ScAccessibleEditLineTextData* pAccTextData;
    BOOL		bIsRTL;
    BOOL		bIsInsertMode;
    BOOL		bFormulaMode;

    // #102710#; this flag should be true if a key input or a command is handled
    // it prevents the call of InputChanged in the ModifyHandler of the EditEngine
    BOOL        bInputMode;
};

//========================================================================

class ScPosWnd : public ComboBox, public SfxListener		// Positionsanzeige
{
private:
    String			aPosStr;
    Accelerator*	pAccel;
    BOOL			bFormulaMode;
    BOOL			bTopHadFocus;

public:
                    ScPosWnd( Window* pParent );
    virtual			~ScPosWnd();


protected:


    virtual void 	SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                            const SfxHint& rHint, const TypeId& rHintType );

private:
    void			FillRangeNames();

};

//========================================================================

class ScInputWindow : public ToolBox						// Parent-Toolbox
{
public:
                    ScInputWindow( Window* pParent, SfxBindings* pBind );
    virtual			~ScInputWindow();

    virtual void 	Resize();






    // Fuer FunktionsAutopiloten



    ScInputHandler*	GetInputHandler(){ return pInputHdl;}

    void			StateChanged( StateChangedType nType );


protected:

    sal_Bool        UseSubTotal( ScRangeList* pRangeList ) const;

private:
    ScPosWnd		aWndPos;
    ScTextWnd		aTextWindow;
    ScInputHandler*	pInputHdl;
    SfxBindings*    pBindings;
    String			aTextOk;
    String			aTextCancel;
    String			aTextSum;
    String			aTextEqual;
    BOOL			bIsOkCancelMode;
};

//==================================================================

class ScInputWindowWrapper : public SfxChildWindow
{
public:
            ScInputWindowWrapper( Window*			pParent,
                                  USHORT			nId,
                                  SfxBindings*		pBindings,
                                  SfxChildWinInfo*	pInfo );

    SFX_DECL_CHILDWINDOW(ScInputWindowWrapper);
};


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
