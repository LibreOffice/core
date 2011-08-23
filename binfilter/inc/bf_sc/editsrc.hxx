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

#ifndef SC_EDITSRC_HXX
#define SC_EDITSRC_HXX

#ifndef SC_SCGLOB_HXX
#include "global.hxx"			// ScRange, ScAddress
#endif

#ifndef _SVX_UNOEDSRC_HXX
#include <bf_svx/unoedsrc.hxx>
#endif

#ifndef _SFXLSTNER_HXX //autogen
#include <bf_svtools/lstner.hxx>
#endif

#include <memory>
namespace binfilter {

class ScEditEngineDefaulter;
class SvxEditEngineForwarder;

class ScDocShell;
class ScHeaderFooterContentObj;
class ScCellTextData;
class ScHeaderFooterTextData;
class ScAccessibleTextData;


class ScHeaderFooterChangedHint : public SfxHint
{
    USHORT nPart;

public:
                    TYPEINFO();
                    ScHeaderFooterChangedHint(USHORT nP);
                    ~ScHeaderFooterChangedHint();

    USHORT			GetPart() const			{ return nPart; }
};


//	all ScSharedHeaderFooterEditSource objects for a single text share the same data

class ScSharedHeaderFooterEditSource : public SvxEditSource
{
private:
    ScHeaderFooterTextData*		pTextData;

protected:
    ScHeaderFooterTextData*		GetTextData() const	{ return pTextData; }	// for ScHeaderFooterEditSource

public:
                                ScSharedHeaderFooterEditSource( ScHeaderFooterTextData* pData );
    virtual						~ScSharedHeaderFooterEditSource();

    //	GetEditEngine is needed because the forwarder doesn't have field functions
    ScEditEngineDefaulter*		GetEditEngine();

    virtual SvxEditSource*		Clone() const ;
    virtual SvxTextForwarder*	GetTextForwarder();

    virtual void				UpdateData();

};

//	ScHeaderFooterEditSource with local copy of ScHeaderFooterTextData is used by field objects

class ScHeaderFooterEditSource : public ScSharedHeaderFooterEditSource
{
public:
                                ScHeaderFooterEditSource( ScHeaderFooterContentObj* pContent, USHORT nP );
                                ScHeaderFooterEditSource( ScHeaderFooterContentObj& rContent, USHORT nP );
    virtual						~ScHeaderFooterEditSource();

    virtual SvxEditSource*		Clone() const;
};


//	Data (incl. EditEngine) for cell EditSource is now shared in ScCellTextData

class ScSharedCellEditSource : public SvxEditSource
{
private:
    ScCellTextData*			pCellTextData;

protected:
    ScCellTextData*			GetCellTextData() const	{ return pCellTextData; }	// for ScCellEditSource

public:
                                ScSharedCellEditSource( ScCellTextData* pData );
    virtual						~ScSharedCellEditSource();

    //	GetEditEngine is needed because the forwarder doesn't have field functions
    ScEditEngineDefaulter*		GetEditEngine();

    virtual SvxEditSource*		Clone() const;
    virtual SvxTextForwarder*	GetTextForwarder();

    virtual void				UpdateData();

    void						SetDoUpdateData(sal_Bool bValue);
    sal_Bool					IsDirty() const;
};

//	ScCellEditSource with local copy of ScCellTextData is used by ScCellFieldsObj, ScCellFieldObj

class ScCellEditSource : public ScSharedCellEditSource
{
public:
                                ScCellEditSource( ScDocShell* pDocSh, const ScAddress& rP );
    virtual						~ScCellEditSource();

    virtual SvxEditSource*		Clone() const;
};


class ScAnnotationEditSource : public SvxEditSource, public SfxListener
{
private:
    ScDocShell*				pDocShell;
    ScAddress				aCellPos;
    ScEditEngineDefaulter*	pEditEngine;
    SvxEditEngineForwarder*	pForwarder;
    BOOL					bDataValid;

public:
                                ScAnnotationEditSource(ScDocShell* pDocSh, const ScAddress& rP);
    virtual						~ScAnnotationEditSource();

    virtual SvxEditSource*		Clone() const ;
    virtual SvxTextForwarder*	GetTextForwarder();
    virtual void				UpdateData();

    virtual void				Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
};


//	EditSource with a shared forwarder for all children of one text object

class ScSimpleEditSource : public SvxEditSource
{
private:
    SvxTextForwarder*	pForwarder;

public:
                        ScSimpleEditSource( SvxTextForwarder* pForw );
    virtual				~ScSimpleEditSource();

    virtual SvxEditSource*		Clone() const ;
    virtual SvxTextForwarder*	GetTextForwarder();
    virtual void				UpdateData();

};

class ScAccessibilityEditSource : public SvxEditSource
{
private:
    ::std::auto_ptr < ScAccessibleTextData > mpAccessibleTextData;

public:
                        ScAccessibilityEditSource( ::std::auto_ptr < ScAccessibleTextData > pAccessibleCellTextData );
    virtual				~ScAccessibilityEditSource();

    virtual SvxEditSource*		Clone() const;
    virtual SvxTextForwarder*	GetTextForwarder();
    virtual SvxViewForwarder*	GetViewForwarder();
    virtual SvxEditViewForwarder*	GetEditViewForwarder( sal_Bool bCreate = sal_False );
    virtual void				UpdateData();
    virtual SfxBroadcaster&		GetBroadcaster() const;
};

} //namespace binfilter
#endif

