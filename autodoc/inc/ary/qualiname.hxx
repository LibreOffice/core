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

#ifndef ARY_QUALINAME_HXX
#define ARY_QUALINAME_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include <cosv/tpl/tpltools.hxx>


namespace ary
{

class QualifiedName
{
  public:
    typedef StringVector::const_iterator  namespace_iterator;

                        QualifiedName(
                            uintt               i_nSize = 0);

    /// @see AssignText()
                        QualifiedName(
                            const char *        i_sText,
                            const char *        i_sSeparator );
                        ~QualifiedName();

    QualifiedName &     operator+=(
                            const String &      i_sNamespaceName )
                                                { if (i_sNamespaceName.length() > 0)
                                                        aNamespace.push_back(i_sNamespaceName);
                                                  return *this; }
    /// @precond i_nIndex < NamespaceDepth().
    String &            operator[](
                            uintt               i_nIndex )
                                                { csv_assert(i_nIndex < aNamespace.size());
                                                  return aNamespace[i_nIndex]; }
    void                Init(
                            bool                i_bAbsolute )
                                                { Empty(); bIsAbsolute = i_bAbsolute; }
    /** Reads a qualified name from a string.
        If the last two characters are "()", the inquiry IsFunction() will return
        true.
    */
    void                AssignText(
                            const char *        i_sText,
                            const char *        i_sSeparator );
    void                SetLocalName(
                            const String &      i_sLocalName )
                                                { sLocalName = i_sLocalName; }
    void                Empty()                 { csv::erase_container(aNamespace); sLocalName.clear(); bIsAbsolute = false; }

    const String &      LocalName() const       { return sLocalName; }
    namespace_iterator  first_namespace() const { return aNamespace.begin(); }
    namespace_iterator  end_namespace() const   { return aNamespace.end(); }
    uintt               NamespaceDepth() const  { return aNamespace.size(); }

    bool                IsAbsolute() const      { return bIsAbsolute; }
    bool                IsQualified() const     { return aNamespace.size() > 0; }
    bool                IsFunction() const      { return bIsFunction; }

  private:
    // DATA
    StringVector        aNamespace;
    String              sLocalName;
    bool                bIsAbsolute;            /// true := beginning with "::".
    bool                bIsFunction;            /// true := ending with "()"
};




}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
