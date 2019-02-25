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

#ifndef INCLUDED_SVX_INC_SDR_CONTACT_VIEWCONTACTOFSDRPAGE_HXX
#define INCLUDED_SVX_INC_SDR_CONTACT_VIEWCONTACTOFSDRPAGE_HXX

#include <sal/types.h>
#include <svx/sdr/contact/viewcontact.hxx>

class SdrPage;

namespace sdr { namespace contact {

class ViewContactOfSdrPage;

class ViewContactOfPageSubObject : public ViewContact
{
    ViewContactOfSdrPage&                       mrParentViewContactOfSdrPage;

public:
    explicit ViewContactOfPageSubObject(ViewContactOfSdrPage& rParentViewContactOfSdrPage);
    virtual ~ViewContactOfPageSubObject() override;

    virtual ViewContact* GetParentContact() const override;
    const SdrPage& getPage() const;
};

class ViewContactOfPageBackground : public ViewContactOfPageSubObject
{
protected:
    virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact) override;
    virtual drawinglayer::primitive2d::Primitive2DContainer createViewIndependentPrimitive2DSequence() const override;

public:
    explicit ViewContactOfPageBackground(ViewContactOfSdrPage& rParentViewContactOfSdrPage);
    virtual ~ViewContactOfPageBackground() override;
};

class ViewContactOfPageShadow : public ViewContactOfPageSubObject
{
protected:
    virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact) override;
    virtual drawinglayer::primitive2d::Primitive2DContainer createViewIndependentPrimitive2DSequence() const override;

public:
    explicit ViewContactOfPageShadow(ViewContactOfSdrPage& rParentViewContactOfSdrPage);
    virtual ~ViewContactOfPageShadow() override;
};

class ViewContactOfPageFill : public ViewContactOfPageSubObject
{
protected:
    virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact) override;
    virtual drawinglayer::primitive2d::Primitive2DContainer createViewIndependentPrimitive2DSequence() const override;

public:
    explicit ViewContactOfPageFill(ViewContactOfSdrPage& rParentViewContactOfSdrPage);
    virtual ~ViewContactOfPageFill() override;
};

class ViewContactOfMasterPage : public ViewContactOfPageSubObject
{
protected:
    virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact) override;
    virtual drawinglayer::primitive2d::Primitive2DContainer createViewIndependentPrimitive2DSequence() const override;

public:
    explicit ViewContactOfMasterPage(ViewContactOfSdrPage& rParentViewContactOfSdrPage);
    virtual ~ViewContactOfMasterPage() override;
};

class ViewContactOfOuterPageBorder : public ViewContactOfPageSubObject
{
protected:
    virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact) override;
    virtual drawinglayer::primitive2d::Primitive2DContainer createViewIndependentPrimitive2DSequence() const override;

public:
    explicit ViewContactOfOuterPageBorder(ViewContactOfSdrPage& rParentViewContactOfSdrPage);
    virtual ~ViewContactOfOuterPageBorder() override;
};

class ViewContactOfInnerPageBorder : public ViewContactOfPageSubObject
{
protected:
    virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact) override;
    virtual drawinglayer::primitive2d::Primitive2DContainer createViewIndependentPrimitive2DSequence() const override;

public:
    explicit ViewContactOfInnerPageBorder(ViewContactOfSdrPage& rParentViewContactOfSdrPage);
    virtual ~ViewContactOfInnerPageBorder() override;
};

/**
 * This view contact corresponds with all SdrObject instances in a single
 * SdrPage.  Its GetObjectCount() returns the number of SdrObject instances
 * in the SdrPage that it represents, and its GetViewContact() returns the
 * view contact of the SdrObject instance associated with the identifier
 * passed to the method.
 */
class ViewContactOfPageHierarchy : public ViewContactOfPageSubObject
{
protected:
    virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact) override;
    virtual drawinglayer::primitive2d::Primitive2DContainer createViewIndependentPrimitive2DSequence() const override;

public:
    explicit ViewContactOfPageHierarchy(ViewContactOfSdrPage& rParentViewContactOfSdrPage);
    virtual ~ViewContactOfPageHierarchy() override;

    virtual sal_uInt32 GetObjectCount() const override;
    virtual ViewContact& GetViewContact(sal_uInt32 nIndex) const override;
};

class ViewContactOfGrid final : public ViewContactOfPageSubObject
{
    bool const                                        mbFront : 1;

    virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact) override;
    virtual drawinglayer::primitive2d::Primitive2DContainer createViewIndependentPrimitive2DSequence() const override;

public:
    ViewContactOfGrid(ViewContactOfSdrPage& rParentViewContactOfSdrPage, bool bFront);
    virtual ~ViewContactOfGrid() override;

    bool getFront() const { return mbFront; }
};

class ViewContactOfHelplines final : public ViewContactOfPageSubObject
{
    bool const                                        mbFront : 1;

    virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact) override;
    virtual drawinglayer::primitive2d::Primitive2DContainer createViewIndependentPrimitive2DSequence() const override;

public:
    ViewContactOfHelplines(ViewContactOfSdrPage& rParentViewContactOfSdrPage, bool bFront);
    virtual ~ViewContactOfHelplines() override;

    bool getFront() const { return mbFront; }
};

class ViewContactOfSdrPage : public ViewContact
{
    // the owner of this ViewContact. Set from constructor and not
    // to be changed in any way.
    SdrPage&                                        mrPage;

    // helper viewContacts to build a clear paint hierarchy
    ViewContactOfPageBackground                     maViewContactOfPageBackground;
    ViewContactOfPageShadow                         maViewContactOfPageShadow;
    ViewContactOfPageFill                           maViewContactOfPageFill;
    ViewContactOfMasterPage                         maViewContactOfMasterPage;
    ViewContactOfOuterPageBorder                    maViewContactOfOuterPageBorder;
    ViewContactOfInnerPageBorder                    maViewContactOfInnerPageBorder;
    ViewContactOfGrid                               maViewContactOfGridBack;
    ViewContactOfHelplines                          maViewContactOfHelplinesBack;
    ViewContactOfPageHierarchy                      maViewContactOfPageHierarchy;
    ViewContactOfGrid                               maViewContactOfGridFront;
    ViewContactOfHelplines                          maViewContactOfHelplinesFront;

protected:
    // Create a Object-Specific ViewObjectContact, set ViewContact and
    // ObjectContact. Always needs to return something. Default is to create
    // a standard ViewObjectContact containing the given ObjectContact and *this
    virtual ViewObjectContact& CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact) override;

public:
    // access to SdrObject
    SdrPage& GetSdrPage() const
    {
        return mrPage;
    }

    // basic constructor, used from SdrPage.
    explicit ViewContactOfSdrPage(SdrPage& rObj);
    virtual ~ViewContactOfSdrPage() override;

    // Access to possible sub-hierarchy
    virtual sal_uInt32 GetObjectCount() const override;
    virtual ViewContact& GetViewContact(sal_uInt32 nIndex) const override;

    // React on changes of the object of this ViewContact
    virtual void ActionChanged() override;

protected:
    // This method is responsible for creating the graphical visualisation data
    // ONLY based on model data
    virtual drawinglayer::primitive2d::Primitive2DContainer createViewIndependentPrimitive2DSequence() const override;
};

}}

#endif // INCLUDED_SVX_INC_SDR_CONTACT_VIEWCONTACTOFSDRPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
