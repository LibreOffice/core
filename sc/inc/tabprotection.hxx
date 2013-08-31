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

#ifndef SC_TAB_PROTECTION_HXX
#define SC_TAB_PROTECTION_HXX

#include "sal/types.h"
#include <com/sun/star/uno/Sequence.hxx>

#include "global.hxx"
#include <boost/shared_ptr.hpp>

class ScDocument;
class ScTableProtectionImpl;

enum ScPasswordHash
{
    PASSHASH_SHA1 = 0,
    PASSHASH_XL,
    PASSHASH_UNSPECIFIED
};

class ScPassHashHelper
{
public:
    /** Check for the compatibility of all password hashes.  If there is at
     * least one hash that needs to be regenerated, it returns true.  If all
     * hash values are compatible with the specified hash type, then it
     * returns false. */
    static bool needsPassHashRegen(const ScDocument& rDoc, ScPasswordHash eHash1, ScPasswordHash eHash2 = PASSHASH_UNSPECIFIED);

    static OUString getHashURI(ScPasswordHash eHash);

    static ScPasswordHash getHashTypeFromURI(const OUString& rURI);

private:
    ScPassHashHelper();
    ~ScPassHashHelper();
};

class SAL_NO_VTABLE ScPassHashProtectable
{
public:
    virtual ~ScPassHashProtectable() = 0;

    virtual bool isProtected() const = 0;
    virtual bool isProtectedWithPass() const = 0;
    virtual void setProtected(bool bProtected) = 0;

    virtual bool isPasswordEmpty() const = 0;
    virtual bool hasPasswordHash(ScPasswordHash eHash, ScPasswordHash eHash2 = PASSHASH_UNSPECIFIED) const = 0;
    virtual void setPassword(const String& aPassText) = 0;
    virtual ::com::sun::star::uno::Sequence<sal_Int8> getPasswordHash(
        ScPasswordHash eHash, ScPasswordHash eHas2 = PASSHASH_UNSPECIFIED) const = 0;
    virtual void setPasswordHash(
        const ::com::sun::star::uno::Sequence<sal_Int8>& aPassword,
        ScPasswordHash eHash = PASSHASH_SHA1, ScPasswordHash eHash2 = PASSHASH_UNSPECIFIED) = 0;
    virtual bool verifyPassword(const String& aPassText) const = 0;
};

class SC_DLLPUBLIC ScDocProtection : public ScPassHashProtectable
{
public:
    enum Option
    {
        STRUCTURE = 0,
        WINDOWS,
        CONTENT,
        NONE        ///< last item - used to resize the vector
    };

    explicit ScDocProtection();
    explicit ScDocProtection(const ScDocProtection& r);
    virtual ~ScDocProtection();

    virtual bool isProtected() const;
    virtual bool isProtectedWithPass() const;
    virtual void setProtected(bool bProtected);

    virtual bool isPasswordEmpty() const;
    virtual bool hasPasswordHash(ScPasswordHash eHash, ScPasswordHash eHash2 = PASSHASH_UNSPECIFIED) const;
    virtual void setPassword(const String& aPassText);
    virtual ::com::sun::star::uno::Sequence<sal_Int8> getPasswordHash(
        ScPasswordHash eHash, ScPasswordHash eHash2 = PASSHASH_UNSPECIFIED) const;
    virtual void setPasswordHash(
        const ::com::sun::star::uno::Sequence<sal_Int8>& aPassword,
        ScPasswordHash eHash = PASSHASH_SHA1, ScPasswordHash eHash2 = PASSHASH_UNSPECIFIED);
    virtual bool verifyPassword(const String& aPassText) const;

    bool isOptionEnabled(Option eOption) const;
    void setOption(Option eOption, bool bEnabled);

private:
    ::boost::shared_ptr<ScTableProtectionImpl> mpImpl;
};

/** sheet protection state container
 *
 * This class stores sheet's protection state: 1) whether the protection
 * is on, 2) password and/or password hash, and 3) any associated
 * protection options.  This class is also used as a protection state
 * container for the undo/redo stack, in which case the password, hash and
 * the options need to be preserved even when the protection flag is
 * off. */
class SC_DLLPUBLIC ScTableProtection : public ScPassHashProtectable
{
public:
    enum Option
    {
        AUTOFILTER = 0,
        DELETE_COLUMNS,
        DELETE_ROWS,
        FORMAT_CELLS,
        FORMAT_COLUMNS,
        FORMAT_ROWS,
        INSERT_COLUMNS,
        INSERT_HYPERLINKS,
        INSERT_ROWS,
        OBJECTS,
        PIVOT_TABLES,
        SCENARIOS,
        SELECT_LOCKED_CELLS,
        SELECT_UNLOCKED_CELLS,
        SHEET,
        SORT,
        NONE        ///< last item - used to resize the vector
    };

    explicit ScTableProtection();
    explicit ScTableProtection(const ScTableProtection& r);
    virtual ~ScTableProtection();

    virtual bool isProtected() const;
    virtual bool isProtectedWithPass() const;
    virtual void setProtected(bool bProtected);

    virtual bool isPasswordEmpty() const;
    virtual bool hasPasswordHash(ScPasswordHash eHash, ScPasswordHash eHash2 = PASSHASH_UNSPECIFIED) const;
    virtual void setPassword(const String& aPassText);
    virtual ::com::sun::star::uno::Sequence<sal_Int8> getPasswordHash(
        ScPasswordHash eHash, ScPasswordHash eHash2 = PASSHASH_UNSPECIFIED) const;
    virtual void setPasswordHash(
        const ::com::sun::star::uno::Sequence<sal_Int8>& aPassword,
        ScPasswordHash eHash = PASSHASH_SHA1, ScPasswordHash eHash2 = PASSHASH_UNSPECIFIED);
    virtual bool verifyPassword(const String& aPassText) const;

    bool isOptionEnabled(Option eOption) const;
    void setOption(Option eOption, bool bEnabled);

private:
    ::boost::shared_ptr<ScTableProtectionImpl> mpImpl;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
