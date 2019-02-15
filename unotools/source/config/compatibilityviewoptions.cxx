/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <unotools/compatibilityviewoptions.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>
#include "itemholder1.hxx"

#define ROOTNODE_COMPATIBILITY_VIEW "Office.Compatibility/View"

#define PROPERTYNAME_MSCOMPATIBLEFORMSMENU "MSCompatibleFormsMenu"

#define PROPERTYHANDLE_MSCOMPATIBLEFORMSMENU 0

class SvtCompatibilityViewOptions_Impl : public utl::ConfigItem
{
private:
    bool m_bShowMSCompatibleFormsMenu;

public:
    SvtCompatibilityViewOptions_Impl();
    virtual ~SvtCompatibilityViewOptions_Impl() override;

    /**
            @short      Called for notify of configmanager.

                        This method is called from the ConfigManager before application ends or from the
                        PropertyChangeListener if the sub tree broadcasts changes. You must update your
                        internal values.

            @see        baseclass ConfigItem

            @param      "seqPropertyNames" is the list of properties which should be updated.
        */
    virtual void Notify(const css::uno::Sequence<OUString>& seqPropertyNames) override;

    bool HasMSOCompatibleFormsMenu() const { return m_bShowMSCompatibleFormsMenu; }
    void SetMSOCompatibleFormsMenu(bool bSet)
    {
        bool bModified = (m_bShowMSCompatibleFormsMenu != bSet);
        if (bModified)
        {
            m_bShowMSCompatibleFormsMenu = bSet;
            SetModified();
            Commit();
        }
    }

private:
    virtual void ImplCommit() override;

    /**
            @short      Return list of fix key names of our configuration management which represent our module tree.

                        This method returns a static const list of key names. We need it to get needed values from our
                        configuration management.

            @return     A list of needed configuration keys is returned.
        */
    static css::uno::Sequence<OUString> const& impl_GetPropertyNames();
};

SvtCompatibilityViewOptions_Impl::SvtCompatibilityViewOptions_Impl()
    : ConfigItem(ROOTNODE_COMPATIBILITY_VIEW)
    , m_bShowMSCompatibleFormsMenu(false)
{
    // Use our static list of configuration keys to get his values.
    css::uno::Sequence<OUString> seqNames = impl_GetPropertyNames();
    css::uno::Sequence<css::uno::Any> seqValues = GetProperties(seqNames);
    assert(seqNames.getLength() == seqValues.getLength());

    if (seqValues[PROPERTYHANDLE_MSCOMPATIBLEFORMSMENU].hasValue())
    {
        assert(seqValues[PROPERTYHANDLE_MSCOMPATIBLEFORMSMENU].getValueTypeClass()
               == css::uno::TypeClass_BOOLEAN);
        seqValues[PROPERTYHANDLE_MSCOMPATIBLEFORMSMENU] >>= m_bShowMSCompatibleFormsMenu;
    }

    EnableNotification(seqNames);
}

SvtCompatibilityViewOptions_Impl::~SvtCompatibilityViewOptions_Impl()
{
    assert(!IsModified()); // should have been committed
}

void SvtCompatibilityViewOptions_Impl::Notify(const css::uno::Sequence<OUString>& seqPropertyNames)
{
    // Use given list of updated properties to get his values from configuration directly!
    css::uno::Sequence<css::uno::Any> seqValues = GetProperties(seqPropertyNames);
    assert(seqPropertyNames.getLength() == seqValues.getLength());

    for (sal_Int32 nProperty = 0; nProperty < seqPropertyNames.getLength(); ++nProperty)
    {
        if (seqPropertyNames[nProperty] == PROPERTYNAME_MSCOMPATIBLEFORMSMENU)
        {
            assert(seqValues[nProperty].getValueTypeClass() == css::uno::TypeClass_BOOLEAN);
            seqValues[nProperty] >>= m_bShowMSCompatibleFormsMenu;
        }
    }
}

void SvtCompatibilityViewOptions_Impl::ImplCommit()
{
    // Get names of supported properties, create a list for values and copy current values to it.
    css::uno::Sequence<OUString> seqNames = impl_GetPropertyNames();
    css::uno::Sequence<css::uno::Any> seqValues(seqNames.getLength());

    seqValues[PROPERTYHANDLE_MSCOMPATIBLEFORMSMENU] <<= m_bShowMSCompatibleFormsMenu;

    // Set properties in configuration.
    PutProperties(seqNames, seqValues);
}

css::uno::Sequence<OUString> const& SvtCompatibilityViewOptions_Impl::impl_GetPropertyNames()
{
    static const css::uno::Sequence<OUString> seqPropertyNames{ OUString(
        PROPERTYNAME_MSCOMPATIBLEFORMSMENU) };
    return seqPropertyNames;
}

namespace
{
std::weak_ptr<SvtCompatibilityViewOptions_Impl> theOptions;
}

SvtCompatibilityViewOptions::SvtCompatibilityViewOptions()
{
    // Global access, must be guarded (multithreading!).
    osl::MutexGuard aGuard(GetOwnStaticMutex());

    m_pImpl = theOptions.lock();
    if (!m_pImpl)
    {
        m_pImpl = std::make_shared<SvtCompatibilityViewOptions_Impl>();
        theOptions = m_pImpl;
        ItemHolder1::holdConfigItem(EItem::CompatibilityView);
    }
}

SvtCompatibilityViewOptions::~SvtCompatibilityViewOptions()
{
    // Global access, must be guarded (multithreading!)
    osl::MutexGuard aGuard(GetOwnStaticMutex());
    m_pImpl.reset();
}

bool SvtCompatibilityViewOptions::HasMSOCompatibleFormsMenu() const
{
    return m_pImpl->HasMSOCompatibleFormsMenu();
}

void SvtCompatibilityViewOptions::SetMSOCompatibleFormsMenu(bool bSet)
{
    m_pImpl->SetMSOCompatibleFormsMenu(bSet);
}

osl::Mutex& SvtCompatibilityViewOptions::GetOwnStaticMutex()
{
    static osl::Mutex ourMutex;

    return ourMutex;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
