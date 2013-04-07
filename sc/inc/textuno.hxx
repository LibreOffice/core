/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef SC_TEXTSUNO_HXX
#define SC_TEXTSUNO_HXX

#include "global.hxx"           // ScRange, ScAddress
#include "address.hxx"
#include <editeng/unotext.hxx>
#include <svl/brdcst.hxx>
#include <svl/lstner.hxx>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/sheet/XHeaderFooterContent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implbase5.hxx>

#include "rtl/ref.hxx"
#include "scdllapi.h"

class EditEngine;
class EditTextObject;
class SvxEditEngineForwarder;
class ScDocShell;
class ScAddress;
class ScCellObj;
class ScSimpleEditSource;
class ScCellEditSource;
class ScEditEngineDefaulter;
class ScFieldEditEngine;
class ScHeaderFooterTextObj;

struct ScHeaderFieldData;


#define SC_HDFT_LEFT    0
#define SC_HDFT_CENTER  1
#define SC_HDFT_RIGHT   2


//  ScHeaderFooterContentObj is a dumb container which must be re-written into
//  the page template using setPropertyValue

class ScHeaderFooterContentObj : public cppu::WeakImplHelper3<
                            com::sun::star::sheet::XHeaderFooterContent,
                            com::sun::star::lang::XUnoTunnel,
                            com::sun::star::lang::XServiceInfo >
{
private:
    rtl::Reference<ScHeaderFooterTextObj> mxLeftText;
    rtl::Reference<ScHeaderFooterTextObj> mxCenterText;
    rtl::Reference<ScHeaderFooterTextObj> mxRightText;

    ScHeaderFooterContentObj(); // disabled

public:
    ScHeaderFooterContentObj( const EditTextObject* pLeft,
                              const EditTextObject* pCenter,
                              const EditTextObject* pRight );
    virtual                 ~ScHeaderFooterContentObj();

                            // for ScPageHFItem (using getImplementation)
    const EditTextObject* GetLeftEditObject() const;
    const EditTextObject* GetCenterEditObject() const;
    const EditTextObject* GetRightEditObject() const;

                            // XHeaderFooterContent
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > SAL_CALL
                            getLeftText() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > SAL_CALL
                            getCenterText() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > SAL_CALL
                            getRightText() throw(::com::sun::star::uno::RuntimeException);

                            // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence<
                                    sal_Int8 >& aIdentifier )
                                throw(::com::sun::star::uno::RuntimeException);

    static const com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScHeaderFooterContentObj* getImplementation( const com::sun::star::uno::Reference<
                                    com::sun::star::sheet::XHeaderFooterContent> xObj );

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


//  ScHeaderFooterTextData: shared data between sub objects of a ScHeaderFooterTextObj

class ScHeaderFooterTextData
{
private:
    EditTextObject* mpTextObj;
    ScHeaderFooterContentObj&   rContentObj;
    sal_uInt16                      nPart;
    ScEditEngineDefaulter*      pEditEngine;
    SvxEditEngineForwarder*     pForwarder;
    bool                        bDataValid;

public:
    ScHeaderFooterTextData(
        ScHeaderFooterContentObj& rContent, sal_uInt16 nP, const EditTextObject* pTextObj);
    ~ScHeaderFooterTextData();

                            // helper functions
    SvxTextForwarder*       GetTextForwarder();
    void UpdateData();
    void UpdateData(EditEngine& rEditEngine);
    ScEditEngineDefaulter*  GetEditEngine() { GetTextForwarder(); return pEditEngine; }

    sal_uInt16                  GetPart() const         { return nPart; }
    ScHeaderFooterContentObj& GetContentObj() const { return rContentObj; }

    const EditTextObject* GetTextObject() const;
};

/**
 * Each of these instances represent, the left, center or right part of the
 * header of footer of a page.
 *
 * ScHeaderFooterTextObj changes the text in a ScHeaderFooterContentObj.
 */
class ScHeaderFooterTextObj : public cppu::WeakImplHelper5<
                            com::sun::star::text::XText,
                            com::sun::star::text::XTextRangeMover,
                            com::sun::star::container::XEnumerationAccess,
                            com::sun::star::text::XTextFieldsSupplier,
                            com::sun::star::lang::XServiceInfo >
{
private:
    ScHeaderFooterTextData      aTextData;
    rtl::Reference<SvxUnoText> mxUnoText;

    void                    CreateUnoText_Impl();

public:
    ScHeaderFooterTextObj(
        ScHeaderFooterContentObj& rContent, sal_uInt16 nP, const EditTextObject* pTextObj);
    virtual ~ScHeaderFooterTextObj();

    const EditTextObject* GetTextObject() const;
    const SvxUnoText&       GetUnoText();

    static void             FillDummyFieldData( ScHeaderFieldData& rData );

                            // XText
    virtual void SAL_CALL   insertTextContent( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::text::XTextRange >& xRange,
                                const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::text::XTextContent >& xContent,
                                sal_Bool bAbsorb )
                                    throw(::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeTextContent( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::text::XTextContent >& xContent )
                                    throw(::com::sun::star::container::NoSuchElementException,
                                            ::com::sun::star::uno::RuntimeException);

                            // XSimpleText
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor > SAL_CALL
                            createTextCursor() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor > SAL_CALL
                            createTextCursorByRange( const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::text::XTextRange >& aTextPosition )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   insertString( const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::text::XTextRange >& xRange,
                                        const OUString& aString, sal_Bool bAbsorb )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   insertControlCharacter( const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::text::XTextRange >& xRange,
                                        sal_Int16 nControlCharacter, sal_Bool bAbsorb )
                                    throw(::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::uno::RuntimeException);

                            // XTextRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > SAL_CALL
                            getText() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL
                            getStart() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL
                            getEnd() throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getString() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setString( const OUString& aString )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XTextRangeMover
    virtual void SAL_CALL   moveTextRange( const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::text::XTextRange >& xRange,
                                        sal_Int16 nParagraphs )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

                            // XTextFieldsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumerationAccess > SAL_CALL
                            getTextFields() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL
                            getTextFieldMasters() throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


//  derived cursor objects for getImplementation and getText/getStart/getEnd

//! uno3: SvxUnoTextCursor is not derived from XUnoTunnel, but should be (?)

class ScCellTextCursor : public SvxUnoTextCursor
{
    ScCellObj&              rTextObj;

public:
                            ScCellTextCursor(const ScCellTextCursor& rOther);
                            ScCellTextCursor(ScCellObj& rText);
        virtual                                 ~ScCellTextCursor() throw();

    ScCellObj&              GetCellObj() const  { return rTextObj; }

                            // SvxUnoTextCursor methods reimplemented here:
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > SAL_CALL
                            getText() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL
                            getStart() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL
                            getEnd() throw(::com::sun::star::uno::RuntimeException);

                            // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence<
                                    sal_Int8 >& aIdentifier )
                                throw(::com::sun::star::uno::RuntimeException);

    static const com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScCellTextCursor* getImplementation( const com::sun::star::uno::Reference<
                                    com::sun::star::uno::XInterface> xObj );
};

class ScHeaderFooterTextCursor : public SvxUnoTextCursor
{
private:
    ScHeaderFooterTextObj&  rTextObj;

public:
                            ScHeaderFooterTextCursor(const ScHeaderFooterTextCursor& rOther);
                            ScHeaderFooterTextCursor(ScHeaderFooterTextObj& rText);
        virtual                                 ~ScHeaderFooterTextCursor() throw();

                            // SvxUnoTextCursor methods reimplemented here:
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > SAL_CALL
                            getText() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL
                            getStart() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL
                            getEnd() throw(::com::sun::star::uno::RuntimeException);

                            // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence<
                                    sal_Int8 >& aIdentifier )
                                throw(::com::sun::star::uno::RuntimeException);

    static const com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScHeaderFooterTextCursor* getImplementation( const com::sun::star::uno::Reference<
                                    com::sun::star::uno::XInterface> xObj );
};

class ScDrawTextCursor : public SvxUnoTextCursor
{
private:
    com::sun::star::uno::Reference< com::sun::star::text::XText > xParentText;

public:
                            ScDrawTextCursor(const ScDrawTextCursor& rOther);
                            ScDrawTextCursor( const com::sun::star::uno::Reference<
                                                com::sun::star::text::XText >& xParent,
                                            const SvxUnoTextBase& rText );
    virtual                  ~ScDrawTextCursor() throw();

                            // SvxUnoTextCursor methods reimplemented here:
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > SAL_CALL
                            getText() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL
                            getStart() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL
                            getEnd() throw(::com::sun::star::uno::RuntimeException);

                            // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence<
                                    sal_Int8 >& aIdentifier )
                                throw(::com::sun::star::uno::RuntimeException);

    static const com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScDrawTextCursor* getImplementation( const com::sun::star::uno::Reference<
                                    com::sun::star::uno::XInterface> xObj );
};


// ScAnnotationTextCursor isn't needed anymore - SvxUnoTextCursor is used instead


//  ScEditEngineTextObj for formatted cell content that is not inserted in a cell or header/footer
//  (used for XML export of change tracking contents)

class ScSimpleEditSourceHelper
{
    ScEditEngineDefaulter*  pEditEngine;
    SvxEditEngineForwarder* pForwarder;
    ScSimpleEditSource*     pOriginalSource;

public:
            ScSimpleEditSourceHelper();
            ~ScSimpleEditSourceHelper();

    ScSimpleEditSource* GetOriginalSource() const   { return pOriginalSource; }
    ScEditEngineDefaulter* GetEditEngine() const    { return pEditEngine; }
};

class ScEditEngineTextObj : public ScSimpleEditSourceHelper, public SvxUnoText
{
public:
                        ScEditEngineTextObj();
        virtual                         ~ScEditEngineTextObj() throw();

    void                SetText( const EditTextObject& rTextObject );
    EditTextObject*     CreateTextObject();
};


//  ScCellTextData: shared data between sub objects of a cell text object

class ScCellTextData : public SfxListener
{
protected:
    ScDocShell*             pDocShell;
    ScAddress               aCellPos;
    ScFieldEditEngine*      pEditEngine;
    SvxEditEngineForwarder* pForwarder;
    ScCellEditSource* pOriginalSource;
    sal_Bool                    bDataValid;
    sal_Bool                    bInUpdate;
    sal_Bool                    bDirty;
    sal_Bool                    bDoUpdate;

protected:
    virtual void            GetCellText(const ScAddress& rCellPos, String& rText);

public:
                            ScCellTextData(ScDocShell* pDocSh, const ScAddress& rP);
    virtual                 ~ScCellTextData();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // helper functions for ScSharedCellEditSource:
    virtual SvxTextForwarder* GetTextForwarder();
    void                    UpdateData();
    ScFieldEditEngine*      GetEditEngine() { GetTextForwarder(); return pEditEngine; }

    ScCellEditSource* GetOriginalSource();        // used as argument for SvxUnoText ctor

                            // used for ScCellEditSource:
    ScDocShell*             GetDocShell() const     { return pDocShell; }
    const ScAddress&        GetCellPos() const      { return aCellPos; }

    void                    SetDirty(sal_Bool bValue)   { bDirty = bValue; }
    sal_Bool                    IsDirty() const         { return bDirty; }
    void                    SetDoUpdate(sal_Bool bValue)    { bDoUpdate = bValue; }
};

class ScCellTextObj : public ScCellTextData, public SvxUnoText
{
public:
                ScCellTextObj(ScDocShell* pDocSh, const ScAddress& rP);
        virtual         ~ScCellTextObj() throw();
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
