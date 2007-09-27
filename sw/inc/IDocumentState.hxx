/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: IDocumentState.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 07:54:15 $
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

 #ifndef IDOCUMENTSTATE_HXX_INCLUDED
 #define IDOCUMENTSTATE_HXX_INCLUDED

 #include <tools/solar.h>

 /** Get information about the current document state
 */
 class IDocumentState
 {
 public:
       /** Bei Formataenderungen muss das zu Fuss gerufen werden!
    */
    virtual void SetModified() = 0;

    /**
    */
    virtual void ResetModified() = 0;

    /** Dokumentaenderungen?
    */
    virtual bool IsModified() const = 0;

    /** Zustaende ueber Ladezustand
        frueher DocInfo
    */
    virtual bool IsLoaded() const = 0;

    /**
    */
    virtual bool IsUpdateExpFld() const  = 0;

    /**
    */
    virtual bool IsNewDoc() const = 0;

    /**
    */
    virtual bool IsPageNums() const = 0;

    /**
    */
    virtual void SetPageNums(bool b)    = 0;

    /**
    */
    virtual void SetNewDoc(bool b) = 0;

    /**
    */
    virtual void SetUpdateExpFldStat(bool b) = 0;

    /**
    */
    virtual void SetLoaded(bool b = sal_True) = 0;

 protected:
    virtual ~IDocumentState() {};
 };

 #endif // IDOCUMENTSTATE_HXX_INCLUDED
