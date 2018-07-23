/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <array>
#include <vector>
#include <unordered_map>

struct XXX {
    ~XXX() {}
};

class Foo1 {
    XXX* m_pbar; // expected-note {{member is here [loplugin:useuniqueptr]}}
    ~Foo1()
    {
        delete m_pbar; // expected-error {{unconditional call to delete on a member, should be using std::unique_ptr [loplugin:useuniqueptr]}}
        m_pbar = nullptr;
    }
};


class Foo2 {
    char* m_pbar1; // expected-note {{member is here [loplugin:useuniqueptr]}}
    char* m_pbar2; // expected-note {{member is here [loplugin:useuniqueptr]}}
    ~Foo2()
    {
        delete[] m_pbar1; // expected-error {{unconditional call to delete on a member, should be using std::unique_ptr [loplugin:useuniqueptr]}}
        delete[] m_pbar2; // expected-error {{unconditional call to delete on a member, should be using std::unique_ptr [loplugin:useuniqueptr]}}
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

class Class4 {
    int* m_pbar[10]; // expected-note {{member is here [loplugin:useuniqueptr]}}
    ~Class4()
    {
        for (int i = 0; i < 10; ++i)
            delete m_pbar[i]; // expected-error {{rather manage with std::some_container<std::unique_ptr<T>> [loplugin:useuniqueptr]}}
    }
};
class Class5 {
    int* m_pbar[10]; // expected-note {{member is here [loplugin:useuniqueptr]}}
    ~Class5()
    {
        for (auto p : m_pbar)
            delete p; // expected-error {{rather manage with std::some_container<std::unique_ptr<T>> [loplugin:useuniqueptr]}}
    }
};
class Class6 {
    std::array<int*,10> m_pbar; // expected-note {{member is here [loplugin:useuniqueptr]}}
    ~Class6()
    {
        for (auto p : m_pbar)
            delete p; // expected-error {{rather manage with std::some_container<std::unique_ptr<T>> [loplugin:useuniqueptr]}}
    }
};
class Class7 {
    std::array<int*,10> m_pbar; // expected-note {{member is here [loplugin:useuniqueptr]}}
    ~Class7()
    {
        for (int i = 0; i < 10; ++i)
            delete m_pbar[i]; // expected-error {{rather manage with std::some_container<std::unique_ptr<T>> [loplugin:useuniqueptr]}}
    }
};
class Class8 {
    std::unordered_map<int, int*> m_pbar; // expected-note {{member is here [loplugin:useuniqueptr]}}
    ~Class8()
    {
        for (auto i : m_pbar)
            delete i.second; // expected-error {{rather manage with std::some_container<std::unique_ptr<T>> [loplugin:useuniqueptr]}}
    }
};
class Foo8 {
    XXX* m_pbar1; // expected-note {{member is here [loplugin:useuniqueptr]}}
    XXX* m_pbar2; // expected-note {{member is here [loplugin:useuniqueptr]}}
    ~Foo8()
    {
        delete m_pbar1; // expected-error {{unconditional call to delete on a member, should be using std::unique_ptr [loplugin:useuniqueptr]}}
        delete m_pbar2; // expected-error {{unconditional call to delete on a member, should be using std::unique_ptr [loplugin:useuniqueptr]}}
    }
};
class Foo9 {
    XXX* m_pbar1; // expected-note {{member is here [loplugin:useuniqueptr]}}
    XXX* m_pbar2; // expected-note {{member is here [loplugin:useuniqueptr]}}
    XXX* m_pbar3; // expected-note {{member is here [loplugin:useuniqueptr]}}
    XXX* m_pbar4; // expected-note {{member is here [loplugin:useuniqueptr]}}
    ~Foo9()
    {
        if (m_pbar1)
        {
            delete m_pbar1; // expected-error {{unconditional call to delete on a member, should be using std::unique_ptr [loplugin:useuniqueptr]}}
        }
        if (m_pbar2 != nullptr)
        {
            delete m_pbar2; // expected-error {{unconditional call to delete on a member, should be using std::unique_ptr [loplugin:useuniqueptr]}}
        }
        if (m_pbar3 != nullptr)
            delete m_pbar3; // expected-error {{unconditional call to delete on a member, should be using std::unique_ptr [loplugin:useuniqueptr]}}
        if (m_pbar4 != nullptr)
        {
            int x = 1;
            (void)x;
            delete m_pbar4; // expected-error {{unconditional call to delete on a member, should be using std::unique_ptr [loplugin:useuniqueptr]}}
        }
    }
};
// no warning expected
class Foo10 {
    XXX* m_pbar1;
    ~Foo10()
    {
        if (m_pbar1 != getOther())
        {
            delete m_pbar1;
        }
    }
    XXX* getOther() { return nullptr; }
};
class Foo11 {
    std::vector<XXX*> m_pbar1; // expected-note {{member is here [loplugin:useuniqueptr]}}
    ~Foo11()
    {
        for (const auto & p : m_pbar1)
        {
            delete p; // expected-error {{rather manage with std::some_container<std::unique_ptr<T>> [loplugin:useuniqueptr]}}
        }
    }
};
class Foo12 {
    std::array<int*,10> m_pbar; // expected-note {{member is here [loplugin:useuniqueptr]}}
    ~Foo12()
    {
        int i = 0;
        while (i < 10)
            delete m_pbar[i++]; // expected-error {{rather manage with std::some_container<std::unique_ptr<T>> [loplugin:useuniqueptr]}}
    }
};
#define DELETEZ( p )    ( delete p,p = NULL )
class Foo13 {
    int * m_pbar1; // expected-note {{member is here [loplugin:useuniqueptr]}}
    int * m_pbar2; // expected-note {{member is here [loplugin:useuniqueptr]}}
    int * m_pbar3; // expected-note {{member is here [loplugin:useuniqueptr]}}
    ~Foo13()
    {
        if (m_pbar1)
            DELETEZ(m_pbar1); // expected-error {{unconditional call to delete on a member, should be using std::unique_ptr [loplugin:useuniqueptr]}}
        DELETEZ(m_pbar2); // expected-error {{unconditional call to delete on a member, should be using std::unique_ptr [loplugin:useuniqueptr]}}
        if (m_pbar3)
        {
            DELETEZ(m_pbar3); // expected-error {{unconditional call to delete on a member, should be using std::unique_ptr [loplugin:useuniqueptr]}}
        }
    }
};
// check for unconditional inner compound statements
class Foo14 {
    int * m_pbar1; // expected-note {{member is here [loplugin:useuniqueptr]}}
    ~Foo14()
    {
        {
            delete m_pbar1; // expected-error {{unconditional call to delete on a member, should be using std::unique_ptr [loplugin:useuniqueptr]}}
        }
    }
};
void Foo15(int * p)
{
    delete p; // expected-error {{calling delete on a pointer param, should be either whitelisted here or simplified [loplugin:useuniqueptr]}}
};
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
