/************************************************************************
 *
 * Licensed Materials - Property of IBM.
 * (C) Copyright IBM Corporation 2003, 2012.  All Rights Reserved.
 * U.S. Government Users Restricted Rights:
 * Use, duplication or disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 ************************************************************************/

package org.openoffice.test.assistant.views;

import static org.openoffice.test.vcl.widgets.VclControl.*;

import java.util.ArrayList;
import java.util.List;

import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IWorkspace;
import org.eclipse.core.resources.IWorkspaceRoot;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.jdt.core.ICompilationUnit;
import org.eclipse.jdt.core.IJavaProject;
import org.eclipse.jdt.core.IPackageFragment;
import org.eclipse.jdt.core.IPackageFragmentRoot;
import org.eclipse.jdt.core.JavaCore;
import org.eclipse.jdt.core.JavaModelException;
import org.eclipse.jdt.core.WorkingCopyOwner;
import org.eclipse.jdt.core.dom.AST;
import org.eclipse.jdt.core.dom.ASTParser;
import org.eclipse.jdt.core.dom.CompilationUnit;
import org.eclipse.jdt.core.dom.FieldDeclaration;
import org.eclipse.jdt.core.dom.MethodInvocation;
import org.eclipse.jdt.core.dom.Modifier.ModifierKeyword;
import org.eclipse.jdt.core.dom.StringLiteral;
import org.eclipse.jdt.core.dom.TypeDeclaration;
import org.eclipse.jdt.core.dom.VariableDeclarationFragment;
import org.eclipse.jface.text.Document;
import org.eclipse.text.edits.TextEdit;
import org.openoffice.test.vcl.IDList;
import org.openoffice.test.vcl.client.SmartId;
@SuppressWarnings({ "unchecked", "rawtypes" })
public class UIMapOp {

    private ICompilationUnit uiMap = null;

    private IDList idList = null;

    public ArrayList<String> names = new ArrayList<String>();

    public ArrayList<String> ids = new ArrayList<String>();

    public UIMapOp() {

    }

    private boolean locateUIMap() {
        uiMap = null;

        IWorkspace workspace = ResourcesPlugin.getWorkspace();
        IWorkspaceRoot root = workspace.getRoot();
        IProject[] projects = root.getProjects();
        for (IProject project : projects) {
            try {
                if (project.isNatureEnabled("org.eclipse.jdt.core.javanature")) {
                    IJavaProject javaProject = JavaCore.create(project);
                    uiMap = findUIMap(javaProject);
                    if (uiMap != null) {
                        return true;
                    }
                }
            } catch (Exception e) {

            }
        }

        return false;
    }

    private ICompilationUnit findUIMap(IJavaProject javaProject) throws JavaModelException {
        IPackageFragment[] packages = javaProject.getPackageFragments();
        for (IPackageFragment mypackage : packages) {
            if (mypackage.getKind() == IPackageFragmentRoot.K_SOURCE) {
                ICompilationUnit ret = findUIMap(mypackage);
                if (ret != null) {
                    return ret;
                }
            }
        }

        return null;
    }

    private ICompilationUnit findUIMap(IPackageFragment mypackage) throws JavaModelException {
        for (ICompilationUnit unit : mypackage.getCompilationUnits()) {
            if ("UIMap.java".equals(unit.getElementName())) {
                return unit;
            }
        }

        return null;
    }

    private static CompilationUnit parse(ICompilationUnit unit) {
        ASTParser parser = ASTParser.newParser(AST.JLS3);
        parser.setKind(ASTParser.K_COMPILATION_UNIT);
        parser.setSource(unit);
        parser.setResolveBindings(true);
        return (CompilationUnit) parser.createAST(null); // parse
    }

    public boolean scan() {
        if (!locateUIMap())
            return false;

        IProject project = uiMap.getResource().getProject();
        try {
            uiMap.getResource().refreshLocal(1, null);
        } catch (CoreException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        IFile file = project.getFile("ids");
        idList = new IDList(file.getLocation().toFile());
        names.clear();
        ids.clear();
        CompilationUnit compilationUnit = parse(uiMap);
        TypeDeclaration typeDeclaration = (TypeDeclaration) compilationUnit.types().get(0);
        FieldDeclaration[] fields = typeDeclaration.getFields();
        for (FieldDeclaration field : fields) {
            List fragments = field.fragments();
            for (Object o : fragments) {
                VariableDeclarationFragment fragment = (VariableDeclarationFragment) o;
                System.out.println(fragment);
                names.add(fragment.getName().toString());

                String code = fragment.toString();
                int left = code.indexOf('(');
                if (left >= 0) {
                    int right = code.indexOf(')', left);
                    if (right >= 0) {
                        String idInDeclaration = code.substring(left + 1, right);
                        if (idInDeclaration.startsWith("\"")) {
                            // It's string....
                            idInDeclaration = idInDeclaration.substring(1, idInDeclaration.length() - 1);
                            ids.add(idInDeclaration);
                            continue;
                        }
                    }
                }

                ids.add(null);

            }
        }

        return true;
    }

    /**
     * Return the control name defined in UIMap with the given id
     *
     * @param id
     * @return
     */
    public void populateName(ArrayList<ControlInfo> controlInfos) {
        for (int i = names.size() - 1; i >= 0; i--) {
            String name = names.get(i);
            String id = ids.get(i);
            if (id == null)
                continue;

            SmartId smartId = idList.getId(id);
            for (ControlInfo ci : controlInfos) {
                if (ci.id.equals(smartId)) {
                    ci.name = name;
                    break;
                }
            }
        }
    }

    /**
     *
     */
    private String[] codeParts(ControlInfo info) {
        String method = null;
        String type = null;
        switch ((int) info.type) {
        case WINDOW_BUTTON:
        case WINDOW_PUSHBUTTON:
        case WINDOW_IMAGEBUTTON:
        case WINDOW_MENUBUTTON:
        case WINDOW_MOREBUTTON:
        case WINDOW_RADIOBUTTON:
        case WINDOW_IMAGERADIOBUTTON:
        case WINDOW_CHECKBOX:
        case WINDOW_TRISTATEBOX:
            method = "button";
            type = "VclButton";
            break;
        case WINDOW_EDIT:
        case WINDOW_MULTILINEEDIT:
        case WINDOW_PATTERNBOX:
        case WINDOW_NUMERICBOX:
        case WINDOW_METRICBOX:
        case WINDOW_CURRENCYBOX:
        case WINDOW_DATEBOX:
        case WINDOW_TIMEBOX:
        case WINDOW_LONGCURRENCYBOX:
            method = "editbox";
            type = "VclEditBox";
            break;
        case WINDOW_COMBOBOX:
            method = "combobox";
            type = "VclComboBox";
            break;
        case WINDOW_LISTBOX:
        case WINDOW_MULTILISTBOX:
        case WINDOW_VALUESETLISTBOX:
            method = "listbox";
            type = "VclListBox";
            break;
        case WINDOW_TABPAGE:
            method = "tabpage";
            type = "VclTabPage";
            break;
        case WINDOW_TOOLBOX:
            method = "toolbox";
            type = "VclToolBox";
            break;

        case WINDOW_TABCONTROL:
            method = "tabcontrol";
            type = "VclTabControl";
            break;

        case WINDOW_WINDOW:
        case WINDOW_BORDERWINDOW:
        case WINDOW_SYSTEMCHILDWINDOW:
            method = "window";
            type = "VclWindow";
            break;
        case WINDOW_DIALOG:
        case WINDOW_MODELESSDIALOG:
        case WINDOW_MODALDIALOG:
        case WINDOW_SYSTEMDIALOG :
        case WINDOW_PATHDIALOG :
        case WINDOW_FILEDIALOG :
        case WINDOW_PRINTERSETUPDIALOG :
        case WINDOW_PRINTDIALOG :
        case WINDOW_COLORDIALOG :
        case WINDOW_FONTDIALOG:
            method = "dialog";
            type = "VclDialog";
            break;
        case WINDOW_DOCKINGWINDOW:
            method = "dockingwin";
            type = "VclDocingWin";
            break;
        case WINDOW_SPINFIELD:
        case WINDOW_PATTERNFIELD:
        case WINDOW_NUMERICFIELD:
        case WINDOW_METRICFIELD:
        case WINDOW_CURRENCYFIELD:
        case WINDOW_DATEFIELD:
        case WINDOW_TIMEFIELD:
        case WINDOW_LONGCURRENCYFIELD:
            method = "field";
            type = "VclField";
            break;
        default:
            method = "control";
            type = "VclControl";
        }
        return new String[] { type, method };
    }

    public void define(ControlInfo info) throws Exception {
        String[] codeParts = codeParts(info);
        String type = codeParts[0];
        String method = codeParts[1];

        String source = uiMap.getSource();
        Document document = new Document(source);

        // creation of DOM/AST from a ICompilationUnit
        ASTParser parser = ASTParser.newParser(AST.JLS3);
        parser.setSource(uiMap);

        CompilationUnit astRoot = (CompilationUnit) parser.createAST(null);
        AST ast = astRoot.getAST();
        astRoot.recordModifications();
        VariableDeclarationFragment vdf = ast.newVariableDeclarationFragment();

        StringLiteral literal = ast.newStringLiteral();
        literal.setLiteralValue(info.id.toString());

        MethodInvocation methodInvocation = ast.newMethodInvocation();
        methodInvocation.setName(ast.newSimpleName(method));
        methodInvocation.arguments().add(literal);

        vdf.setInitializer(methodInvocation);
        vdf.setName(ast.newSimpleName(info.name));

        FieldDeclaration fieldDeclaration = ast.newFieldDeclaration(vdf);
        fieldDeclaration.setType(ast.newSimpleType(ast.newSimpleName(type)));

        fieldDeclaration.modifiers().add(ast.newModifier(ModifierKeyword.PUBLIC_KEYWORD));
        fieldDeclaration.modifiers().add(ast.newModifier(ModifierKeyword.STATIC_KEYWORD));
        fieldDeclaration.modifiers().add(ast.newModifier(ModifierKeyword.FINAL_KEYWORD));

        TypeDeclaration typeDeclaration = (TypeDeclaration) astRoot.types().get(0);

        typeDeclaration.bodyDeclarations().add(fieldDeclaration);

        TextEdit edits = astRoot.rewrite(document, uiMap.getJavaProject().getOptions(true));

        edits.apply(document);
        String newSource = document.get();
        WorkingCopyOwner owner = new WorkingCopyOwner() {};

        // Create working copy
        ICompilationUnit workingCopy = uiMap.getWorkingCopy(owner, null);
        workingCopy.getBuffer().setContents(newSource);
        workingCopy.reconcile(ICompilationUnit.NO_AST, false, null, null);
        // Commit changes
        workingCopy.commitWorkingCopy(true, null);
        // Destroy working copy
        workingCopy.discardWorkingCopy();
        // return "public static final " + type + " " + info.name + " = " +
        // method + "(\"" + info.id + "\")";
    }
}
