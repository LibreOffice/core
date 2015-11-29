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

#ifndef INCLUDED_SC_INC_TEXTUNO_HXX
#define INCLUDED_SC_INC_TEXTUNO_HXX

#include "global.hxx"
#include "address.hxx"
#include <editeng/unotext.hxx>
#include <svl/lstner.hxx>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/sheet/XHeaderFooterContent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/implbase.hxx>

#include <rtl/ref.hxx>
#include "scdllapi.h"

#include <boost/noncopyable.hpp>

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

class ScHeaderFooterContentObj : public cppu::WeakImplHelper<
                            css::sheet::XHeaderFooterContent,
                            css::lang::XUnoTunnel,
                            css::lang::XServiceInfo >
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
    virtual css::uno::Reference< css::text::XText > SAL_CALL
                            getLeftText() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::text::XText > SAL_CALL
                            getCenterText() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::text::XText > SAL_CALL
                            getRightText() throw(css::uno::RuntimeException, std::exception) override;

                            // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence<
                                    sal_Int8 >& aIdentifier )
                                throw(css::uno::RuntimeException, std::exception) override;

    static const css::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static rtl::Reference<ScHeaderFooterContentObj> getImplementation(const css::uno::Reference<css::sheet::XHeaderFooterContent>& rObj);

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;

    virtual void dispose();
};

//  ScHeaderFooterTextData: shared data between sub objects of a ScHeaderFooterTextObj

class ScHeaderFooterTextData : boost::noncopyable
{
private:
    EditTextObject* mpTextObj;
    rtl::Reference<ScHeaderFooterContentObj> rContentObj;
    sal_uInt16                      nPart;
    ScEditEngineDefaulter*      pEditEngine;
    SvxEditEngineForwarder*     pForwarder;
    bool                        bDataValid;

public:
    ScHeaderFooterTextData(
        rtl::Reference<ScHeaderFooterContentObj> const & rContent, sal_uInt16 nP, const EditTextObject* pTextObj);
    ~ScHeaderFooterTextData();

                            // helper functions
    SvxTextForwarder*       GetTextForwarder();
    void UpdateData();
    void UpdateData(EditEngine& rEditEngine);
    ScEditEngineDefaulter*  GetEditEngine() { GetTextForwarder(); return pEditEngine; }

    sal_uInt16                  GetPart() const         { return nPart; }
    rtl::Reference<ScHeaderFooterContentObj> GetContentObj() const { return rContentObj; }

    const EditTextObject* GetTextObject() const { return mpTextObj; }

    void dispose() { rContentObj.clear(); }
};

/**
 * Each of these instances represent, the left, center or right part of the
 * header of footer of a page.
 *
 * ScHeaderFooterTextObj changes the text in a ScHeaderFooterContentObj.
 */
class ScHeaderFooterTextObj : public cppu::WeakImplHelper<
                            css::text::XText,
                            css::text::XTextRangeMover,
                            css::container::XEnumerationAccess,
                            css::text::XTextFieldsSupplier,
                            css::lang::XServiceInfo >
{
private:
    ScHeaderFooterTextData      aTextData;
    rtl::Reference<SvxUnoText> mxUnoText;

    void                    CreateUnoText_Impl();

public:
    ScHeaderFooterTextObj(
        rtl::Reference<ScHeaderFooterContentObj> const & rContent, sal_uInt16 nP, const EditTextObject* pTextObj);
    virtual ~ScHeaderFooterTextObj();

    const EditTextObject* GetTextObject() const;
    const SvxUnoText&       GetUnoText();

    static void             FillDummyFieldData( ScHeaderFieldData& rData );

                            // XText
    virtual void SAL_CALL   insertTextContent( const css::uno::Reference< css::text::XTextRange >& xRange,
                                const css::uno::Reference< css::text::XTextContent >& xContent,
                                sal_Bool bAbsorb )
                                    throw(css::lang::IllegalArgumentException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeTextContent( const css::uno::Reference< css::text::XTextContent >& xContent )
                                    throw(css::container::NoSuchElementException,
                                            css::uno::RuntimeException, std::exception) override;

                            // XSimpleText
    virtual css::uno::Reference< css::text::XTextCursor > SAL_CALL
                            createTextCursor() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::text::XTextCursor > SAL_CALL
                            createTextCursorByRange( const css::uno::Reference< css::text::XTextRange >& aTextPosition )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   insertString( const css::uno::Reference< css::text::XTextRange >& xRange,
                                        const OUString& aString, sal_Bool bAbsorb )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   insertControlCharacter( const css::uno::Reference< css::text::XTextRange >& xRange,
                                        sal_Int16 nControlCharacter, sal_Bool bAbsorb )
                                    throw(css::lang::IllegalArgumentException,
                                        css::uno::RuntimeException, std::exception) override;

                            // XTextRange
    virtual css::uno::Reference< css::text::XText > SAL_CALL
                            getText() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL
                            getStart() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL
                            getEnd() throw(css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getString() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setString( const OUString& aString )
                                    throw(css::uno::RuntimeException, std::exception) override;

                            // XTextRangeMover
    virtual void SAL_CALL   moveTextRange( const css::uno::Reference< css::text::XTextRange >& xRange,
                                        sal_Int16 nParagraphs )
                                    throw(css::uno::RuntimeException, std::exception) override;

                            // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(css::uno::RuntimeException, std::exception) override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

                            // XTextFieldsSupplier
    virtual css::uno::Reference< css::container::XEnumerationAccess > SAL_CALL
                            getTextFields() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL
                            getTextFieldMasters() throw(css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;

    virtual void dispose() { aTextData.dispose(); }
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
    virtual css::uno::Reference< css::text::XText > SAL_CALL
                            getText() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL
                            getStart() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL
                            getEnd() throw(css::uno::RuntimeException, std::exception) override;

                            // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier )
                                throw(css::uno::RuntimeException, std::exception) override;

    static const css::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScCellTextCursor* getImplementation(const css::uno::Reference< css::uno::XInterface>& rObj);
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
    virtual css::uno::Reference< css::text::XText > SAL_CALL
                            getText() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL
                            getStart() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL
                            getEnd() throw(css::uno::RuntimeException, std::exception) override;

                            // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier )
                                throw(css::uno::RuntimeException, std::exception) override;

    static const css::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScHeaderFooterTextCursor* getImplementation(const css::uno::Reference<css::uno::XInterface>& rObj);
};

class ScDrawTextCursor : public SvxUnoTextCursor
{
private:
    css::uno::Reference< css::text::XText > xParentText;

public:
                            ScDrawTextCursor(const ScDrawTextCursor& rOther);
                            ScDrawTextCursor( const css::uno::Reference< css::text::XText >& xParent,
                                            const SvxUnoTextBase& rText );
    virtual                  ~ScDrawTextCursor() throw();

                            // SvxUnoTextCursor methods reimplemented here:
    virtual css::uno::Reference< css::text::XText > SAL_CALL
                            getText() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL
                            getStart() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL
                            getEnd() throw(css::uno::RuntimeException, std::exception) override;

                            // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier )
                                throw(css::uno::RuntimeException, std::exception) override;

    static const css::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScDrawTextCursor* getImplementation(const css::uno::Reference<css::uno::XInterface>& rObj);
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
    bool                    bDataValid;
    bool                    bInUpdate;
    bool                    bDirty;
    bool                    bDoUpdate;

protected:
    virtual void            GetCellText(const ScAddress& rCellPos, OUString& rText);

public:
                            ScCellTextData(ScDocShell* pDocSh, const ScAddress& rP);
    virtual                 ~ScCellTextData();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // helper functions for ScSharedCellEditSource:
    virtual SvxTextForwarder* GetTextForwarder();
    void                    UpdateData();
    ScFieldEditEngine*      GetEditEngine() { GetTextForwarder(); return pEditEngine; }

    ScCellEditSource* GetOriginalSource();        // used as argument for SvxUnoText ctor

                            // used for ScCellEditSource:
    ScDocShell*             GetDocShell() const     { return pDocShell; }
    const ScAddress&        GetCellPos() const      { return aCellPos; }

    bool                    IsDirty() const         { return bDirty; }
    void                    SetDoUpdate(bool bValue)    { bDoUpdate = bValue; }
};

class ScCellTextObj : public ScCellTextData, public SvxUnoText
{
public:
                ScCellTextObj(ScDocShell* pDocSh, const ScAddress& rP);
        virtual         ~ScCellTextObj() throw();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
