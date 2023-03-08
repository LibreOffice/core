/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string>
#include <iostream>

#include "check.hxx"
#include "plugin.hxx"
#include "config_clang.h"
#include "clang/AST/CXXInheritance.h"

/**

Look for places where we are using std::unique_ptr to hold a small object,
where we should rather be using std::optional.

*/

namespace
{
class Unique2Optional : public loplugin::FilteringPlugin<Unique2Optional>
{
public:
    explicit Unique2Optional(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual bool preRun() override { return true; }

    virtual void run() override
    {
        if (preRun())
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitFieldDecl(const FieldDecl*);
    bool VisitVarDecl(const VarDecl*);

private:
    bool doDecl(const DeclaratorDecl*);
    bool isSmall(QualType type);
};

bool Unique2Optional::VisitFieldDecl(const FieldDecl* fieldDecl) { return doDecl(fieldDecl); }
bool Unique2Optional::VisitVarDecl(const VarDecl*)
{
    return true; //doDecl(varDecl);
}

bool Unique2Optional::doDecl(const DeclaratorDecl* fieldDecl)
{
    if (ignoreLocation(fieldDecl))
        return true;

    SourceLocation spellingLocation
        = compiler.getSourceManager().getSpellingLoc(fieldDecl->getBeginLoc());
    StringRef fileName = getFilenameOfLocation(spellingLocation);

    // pimpl pattern
    if (loplugin::isSamePathname(fileName, SRCDIR "/include/unotools/closeveto.hxx"))
        return true;
    if (loplugin::isSamePathname(fileName, SRCDIR "/include/svl/svdde.hxx"))
        return true;
    if (loplugin::isSamePathname(fileName, SRCDIR "/include/vcl/toolkit/morebtn.hxx"))
        return true;
    if (loplugin::isSamePathname(fileName, SRCDIR "/include/vcl/toolkit/morebtn.hxx"))
        return true;
    if (loplugin::isSamePathname(fileName, SRCDIR "/include/xmloff/xmlexp.hxx"))
        return true;
    if (loplugin::isSamePathname(fileName, SRCDIR "/include/xmloff/txtparae.hxx"))
        return true;
    if (loplugin::isSamePathname(fileName, SRCDIR "/include/xmloff/controlpropertyhdl.hxx"))
        return true;
    if (loplugin::isSamePathname(fileName, SRCDIR "/cui/source/inc/cuitabarea.hxx"))
        return true;

    // std::type_info is not movable or copyable
    if (loplugin::isSamePathname(fileName,
                                 SRCDIR "/bridges/source/cpp_uno/gcc3_linux_x86-64/rtti.cxx"))
        return true;

    // TODO not sure what is going on here, get a compile error
    if (loplugin::isSamePathname(fileName, SRCDIR "/vcl/inc/unx/printerjob.hxx"))
        return true;

    // Seems in bad taste to modify these
    if (loplugin::isSamePathname(fileName, SRCDIR "/cui/source/tabpages/macroass.cxx"))
        return true;
    if (loplugin::isSamePathname(fileName, SRCDIR "/cui/source/inc/cuitabline.hxx"))
        return true;
    if (loplugin::isSamePathname(fileName, SRCDIR "/cui/source/inc/page.hxx"))
        return true;
    if (loplugin::isSamePathname(fileName, SRCDIR "/sd/source/ui/sidebar/SlideBackground.hxx"))
        return true;
    if (loplugin::isSamePathname(fileName, SRCDIR "/hwpfilter/source/nodes.h"))
        return true;
    if (loplugin::isSamePathname(fileName, SRCDIR "/hwpfilter/source/hwpfile.h"))
        return true;
    if (loplugin::isSamePathname(fileName, SRCDIR "/sw/source/uibase/inc/bookmark.hxx"))
        return true;
    if (loplugin::isSamePathname(fileName, SRCDIR "/sw/inc/viewsh.hxx"))
        return true;
    if (loplugin::isSamePathname(fileName, SRCDIR "/sw/source/uibase/sidebar/PageFormatPanel.hxx"))
        return true;
    if (loplugin::isSamePathname(fileName, SRCDIR "/sw/source/uibase/sidebar/PageStylesPanel.hxx"))
        return true;
    if (loplugin::isSamePathname(fileName, SRCDIR "/sc/inc/dpsave.hxx"))
        return true;
    if (loplugin::isSamePathname(fileName, SRCDIR "/sc/source/ui/inc/dpgroupdlg.hxx"))
        return true;
    if (loplugin::isSamePathname(fileName, SRCDIR "/sc/source/ui/inc/pvfundlg.hxx"))
        return true;
    if (loplugin::isSamePathname(fileName, SRCDIR "/sc/inc/document.hxx"))
        return true;
    if (loplugin::isSamePathname(fileName, SRCDIR "/sc/inc/scmod.hxx"))
        return true;
    if (loplugin::isSamePathname(fileName, SRCDIR "/include/svx/gallery1.hxx"))
        return true;
    if (loplugin::isSamePathname(fileName, SRCDIR "/svx/inc/textchainflow.hxx"))
        return true;
    if (loplugin::isSamePathname(fileName, SRCDIR "/include/svx/graphctl.hxx"))
        return true;
    if (loplugin::isSamePathname(fileName, SRCDIR "/include/svx/float3d.hxx"))
        return true;
    if (loplugin::isSamePathname(fileName, SRCDIR "/dbaccess/source/ui/dlg/generalpage.hxx"))
        return true;
    if (loplugin::isSamePathname(fileName,
                                 SRCDIR "/extensions/source/propctrlr/cellbindinghandler.hxx"))
        return true;
    if (loplugin::isSamePathname(fileName, SRCDIR "/sc/inc/chartlis.hxx"))
        return true;

    // header ordering issues make this hard to change
    if (loplugin::isSamePathname(fileName, SRCDIR "/lotuswordpro/source/filter/lwpdlvlist.hxx"))
        return true;

    // the classes being allocate are ref-counted
    if (loplugin::isSamePathname(fileName, SRCDIR "/sc/source/filter/inc/xeextlst.hxx"))
        return true;
    if (loplugin::isSamePathname(fileName, SRCDIR "/sc/source/filter/inc/xestyle.hxx"))
        return true;
    if (loplugin::isSamePathname(fileName, SRCDIR "/sc/source/ui/inc/tpcalc.hxx"))
        return true;

    // not sure
    if (fileName.contains("QtInstance.hxx")) // "/vcl/inc/qt5/QtInstance.hxx"))
        return true;

    // class is defined inside the module, typically some kind of child/pimpl/listener thing
    if (loplugin::isSamePathname(fileName, SRCDIR "/include/sfx2/viewfrm.hxx"))
        return true;
    if (loplugin::isSamePathname(fileName, SRCDIR "/include/svx/linectrl.hxx"))
        return true;
    if (loplugin::isSamePathname(fileName, SRCDIR "/include/svx/sidebar/LinePropertyPanelBase.hxx"))
        return true;
    if (loplugin::isSamePathname(fileName, SRCDIR "/vcl/source/fontsubset/ttcr.hxx"))
        return true;
    if (loplugin::isSamePathname(fileName,
                                 SRCDIR "/extensions/source/propctrlr/defaultforminspection.hxx"))
        return true;
    if (loplugin::isSamePathname(fileName,
                                 SRCDIR "/extensions/source/propctrlr/propertyhandler.hxx"))
        return true;
    if (loplugin::isSamePathname(fileName, SRCDIR "/sc/inc/spellcheckcontext.hxx"))
        return true;
    if (loplugin::isSamePathname(fileName, SRCDIR "/sw/source/core/inc/layouter.hxx"))
        return true;
    if (loplugin::isSamePathname(fileName, SRCDIR "/sw/source/uibase/inc/numberingtypelistbox.hxx"))
        return true;
    if (loplugin::isSamePathname(fileName, SRCDIR "/lotuswordpro/inc/xfilter/xfdrawstyle.hxx"))
        return true;
    if (loplugin::isSamePathname(fileName, SRCDIR "/sc/source/ui/inc/scuiimoptdlg.hxx"))
        return true;

    // One of the constructors initialises the field by receiving an unique_ptr
    if (loplugin::isSamePathname(fileName,
                                 SRCDIR "/lotuswordpro/source/filter/lwpbreaksoverride.hxx"))
        return true;
    if (loplugin::isSamePathname(fileName,
                                 SRCDIR "/lotuswordpro/source/filter/lwpcharborderoverride.hxx"))
        return true;
    if (loplugin::isSamePathname(fileName,
                                 SRCDIR "/lotuswordpro/source/filter/lwpparaborderoverride.hxx"))
        return true;

    if (!loplugin::TypeCheck(fieldDecl->getType()).ClassOrStruct("unique_ptr").StdNamespace())
        return true;

    auto templateDecl = dyn_cast_or_null<ClassTemplateSpecializationDecl>(
        fieldDecl->getType()->getAsRecordDecl());
    if (!templateDecl)
        return true;
    if (templateDecl->getTemplateArgs().size() == 0)
        return true;
    auto firstTemplateParamType = templateDecl->getTemplateArgs()[0].getAsType();
    if (!isSmall(firstTemplateParamType))
        return true;
    auto paramRecordDecl = firstTemplateParamType->getAsCXXRecordDecl();
    if (paramRecordDecl)
    {
        // if the pointed-to type has a virtual destructor, then we don't know for sure
        // what size type will be stored there
        if (!paramRecordDecl->isEffectivelyFinal())
            if (CXXDestructorDecl* dd = paramRecordDecl->getDestructor())
                if (dd->isVirtual())
                    return true;
        // If it doesn't have a move constructor, then it would be hard to assign into it
        // TODO this condition could be relaxed in some situations
        if (!paramRecordDecl->hasMoveConstructor())
            return true;
        // the weld stuff needs to be heap allocated
        if (loplugin::DeclCheck(paramRecordDecl).Class("CustomWeld").Namespace("weld"))
            return true;
        // ref-counted classes should be heap allocated
        if (loplugin::DeclCheck(paramRecordDecl).Class("SvxContourItem"))
            return true;
        if (loplugin::DeclCheck(paramRecordDecl).Class("SvxAdjustItem"))
            return true;
        if (loplugin::DeclCheck(paramRecordDecl).Class("SwFormatNoBalancedColumns"))
            return true;
        if (loplugin::DeclCheck(paramRecordDecl).Class("SwFormatFollowTextFlow"))
            return true;
    }
    // ignore pimpl pattern
    if (fieldDecl->getName().contains("pImpl"))
        return true;
    if (fieldDecl->getName().contains("impl_"))
        return true;
    if (fieldDecl->getName().contains("mxImpl"))
        return true;
    if (fieldDecl->getName().contains("m_aImpl"))
        return true;
    report(DiagnosticsEngine::Warning, "can use std::optional here, heap-stored type is very small",
           fieldDecl->getLocation())
        << fieldDecl->getSourceRange();
    if (paramRecordDecl)
        report(DiagnosticsEngine::Note, "class being allocated is here",
               paramRecordDecl->getLocation())
            << paramRecordDecl->getSourceRange();
    return true;
}

bool Unique2Optional::isSmall(QualType type)
{
    if (type->isIncompleteType())
        return false;
    clang::Type const* t2 = type.getTypePtrOrNull();
    if (!t2)
        return false;
    // 8 bytes == 1 pointer on 64-bit CPU
    return compiler.getASTContext().getTypeSizeInChars(t2).getQuantity() <= 16;
}

loplugin::Plugin::Registration<Unique2Optional> unique2optional("unique2optional");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
