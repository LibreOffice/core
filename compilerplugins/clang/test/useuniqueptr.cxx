/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


class Foo1 {
    char* m_pbar; // expected-note {{member is here [loplugin:useuniqueptr]}}
    ~Foo1()
    {
        delete m_pbar; // expected-error {{a destructor with only a single unconditional call to delete on a member, is a sure sign it should be using std::unique_ptr for that field [loplugin:useuniqueptr]}}
        m_pbar = nullptr;
    }
};


class Foo2 {
    char* m_pbar1; // expected-note {{member is here [loplugin:useuniqueptr]}}
    char* m_pbar2; // expected-note {{member is here [loplugin:useuniqueptr]}}
    ~Foo2()
    {
        delete[] m_pbar1; // expected-error {{managing array of trival type 'char' manually, rather use std::vector / std::array / std::unique_ptr [loplugin:useuniqueptr]}}
        delete[] m_pbar2; // expected-error {{managing array of trival type 'char' manually, rather use std::vector / std::array / std::unique_ptr [loplugin:useuniqueptr]}}
    }
};

class Foo3 {
    char* m_pbar;
    bool bMine;
    ~Foo3()
    {
        if (bMine)
            delete[] m_pbar;
    }
};
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
