#!/usr/bin/env python3

# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

"""
Script to generate https://wiki.documentfoundation.org/Development/DispatchCommands
3 types of source files are scanned to identify and describe a list of relevant UNO commands:
- .hxx files: containing the symbolic and numeric id's, and the respective modes and groups
- .xcu files; containing several english labels as they appear in menus or tooltips
- .sdi files: containing a list of potential arguments for the commands, and their types
"""

import os
import sys

# It is assumed that the script is called from $BUILDDIR;
# and __file__ refers to the script location in $SRCDIR.
# This allows to use it in separate builddir configuration.
srcdir = os.path.dirname(os.path.realpath(__file__)) + '/..' # go up from /bin
builddir = os.getcwd()

REPO = 'https://opengrok.libreoffice.org/xref/core'

BLACKLIST = ('_SwitchViewShell0', '_SwitchViewShell1', '_SwitchViewShell2', '_SwitchViewShell3', '_SwitchViewShell4')

XCU_DIR = srcdir + '/officecfg/registry/data/org/openoffice/Office/UI/'
XCU_FILES = (   XCU_DIR + 'BasicIDECommands.xcu',
                XCU_DIR + 'CalcCommands.xcu',
                XCU_DIR + 'ChartCommands.xcu',
                XCU_DIR + 'DbuCommands.xcu',
                XCU_DIR + 'DrawImpressCommands.xcu',
                XCU_DIR + 'GenericCommands.xcu',
                XCU_DIR + 'MathCommands.xcu',
                XCU_DIR + 'ReportCommands.xcu',
                XCU_DIR + 'WriterCommands.xcu')

HXX_DIR = builddir + '/workdir/SdiTarget/'
HXX_FILES = (   HXX_DIR + 'basctl/sdi/basslots.hxx',
                HXX_DIR + 'sc/sdi/scslots.hxx',
                HXX_DIR + 'sd/sdi/sdgslots.hxx',
                HXX_DIR + 'sd/sdi/sdslots.hxx',
                HXX_DIR + 'sfx2/sdi/sfxslots.hxx',
                HXX_DIR + 'starmath/sdi/smslots.hxx',
                HXX_DIR + 'svx/sdi/svxslots.hxx',
                HXX_DIR + 'sw/sdi/swslots.hxx')

SDI_FILES = (   srcdir + '/sc/sdi/scalc.sdi',
                srcdir + '/sd/sdi/sdraw.sdi',
                srcdir + '/sfx2/sdi/sfx.sdi',
                srcdir + '/starmath/sdi/smath.sdi',
                srcdir + '/svx/sdi/svx.sdi',
                srcdir + '/sw/sdi/swriter.sdi')

# Category is defined by the 1st file where the command has been found. Precedence: 1. xcu, 2. hxx, 3. sdi.
MODULES = {'BasicIDE':      'Basic IDE, Forms, Dialogs',
            'Calc':         'Calc',
            'Chart':        'Charts',
            'Dbu':          'Base',
            'DrawImpress':  'Draw / Impress',
            'Generic':      'Global',
            'Math':         'Math',
            'Report':       'Reports',
            'Writer':       'Writer',
            'basslots':     'Basic IDE, Forms, Dialogs',
            'scslots':      'Calc',
            'sdgslots':     'Draw / Impress',
            'sdslots':      'Draw / Impress',
            'sfxslots':     'Global',
            'smslots':      'Math',
            'svxslots':     'Global',
            'swslots':      'Writer',
            'scalc':        'Calc',
            'sdraw':        'Draw / Impress',
            'sfx':          'Global',
            'smath':        'Math',
            'svx':          'Global',
            'swriter':      'Writer'}

def newcommand(unocommand):
    cmd = {'unocommand': unocommand,
           'module': '',
           'xcufile': -1,
           'xculinenumber': 0,
           'xcuoccurs': 0,
           'label': '',
           'contextlabel': '',
           'tooltiplabel': '',
           'hxxfile': -1,
           'hxxoccurs': 0,
           'hxxlinenumber': 0,
           'resourceid': '',
           'numericid': '',
           'group': '',
           'sdifile': -1,
           'sdioccurs': 0,
           'sdilinenumber': 0,
           'mode': '',
           'arguments': ''}
    return cmd

def get_uno(cmd):
    if cmd.startswith('FocusToFindbar'):
        cmd = 'vnd.sun.star.findbar:' + cmd
    else:
        cmd = '.uno:' + cmd
    return cmd

def analyze_xcu(all_commands):
    for filename in XCU_FILES:
        ln = 0
        with open(filename) as fh:
            popups = False
            for line in fh:
                ln += 1
                if '<node oor:name="Popups">' in line:
                    popups = True
                    continue
                elif popups is True and line == '    </node>':
                    popups = False
                    continue
                if '<node oor:name=".uno:' not in line and '<node oor:name="vnd.' not in line:
                    continue

                cmdln = ln
                tmp = line.split('"')
                command_name = tmp[1]

                while '</node>' not in line:
                    try:
                        line = next(fh)
                        ln += 1
                    except StopIteration:
                        print("Warning: couldn't find '</node>' line in %s" % filename,
                            file=sys.stderr)
                        break
                    if '<prop oor:name="Label"' in line:
                        label = 'label'
                    elif '<prop oor:name="ContextLabel"' in line:
                        label = 'contextlabel'
                    elif '<prop oor:name="TooltipLabel"' in line:
                        label = 'tooltiplabel'
                    elif '<value xml:lang="en-US">' in line:
                        labeltext = line.replace('<value xml:lang="en-US">', '').replace('</value>', '').strip()

                if popups is False:
                    if command_name not in all_commands:
                        all_commands[command_name] = newcommand(command_name)
                    #
                    all_commands[command_name]['xcufile'] = XCU_FILES.index(filename)
                    all_commands[command_name]['xculinenumber'] = cmdln
                    all_commands[command_name][label] = labeltext.replace('~', '')
                    all_commands[command_name]['xcuoccurs'] += 1


def analyze_hxx(all_commands):
    for filename in HXX_FILES:
        with open(filename) as fh:
            ln = 0
            mode = ''
            for line in fh:
                ln += 1
                if not line.startswith('// Slot Nr. '):
                    continue

                # Parse sth like
                # // Slot Nr. 0 : 5502
                # SFX_NEW_SLOT_ARG( basctl_Shell,SID_SAVEASDOC,SfxGroupId::Document,
                cmdln = ln
                tmp = line.split(':')
                command_id = tmp[1].strip()

                line = next(fh)
                ln += 1
                tmp = line.split(',')
                command_rid = tmp[1]
                command_group = tmp[2].split('::')[1]

                next(fh)
                ln += 1
                next(fh)
                ln += 1
                line = next(fh)
                ln += 1
                mode += 'U' if 'AUTOUPDATE' in line else ''
                mode += 'M' if 'MENUCONFIG' in line else ''
                mode += 'T' if 'TOOLBOXCONFIG' in line else ''
                mode += 'A' if 'ACCELCONFIG' in line else ''

                next(fh)
                ln += 1
                next(fh)
                ln += 1
                line = next(fh)
                ln += 1
                if '"' not in line:
                    line = next(fh)
                tmp = line.split('"')
                try:
                    command_name = get_uno(tmp[1])
                except IndexError:
                    print("Warning: expected \" in line '%s' from file %s" % (line.strip(), filename),
                            file=sys.stderr)
                    command_name = '.uno:'

                if command_name not in all_commands:
                    all_commands[command_name] = newcommand(command_name)
                #
                all_commands[command_name]['hxxfile'] = HXX_FILES.index(filename)
                all_commands[command_name]['hxxlinenumber'] = cmdln
                all_commands[command_name]['numericid'] = command_id
                all_commands[command_name]['resourceid'] = command_rid
                all_commands[command_name]['group'] = command_group
                all_commands[command_name]['mode'] = mode
                all_commands[command_name]['hxxoccurs'] += 1
                mode = ''


def analyze_sdi(all_commands):
    def SplitArguments(params):
        # Split a string like : SfxStringItem Name SID_CHART_NAME,SfxStringItem Range SID_CHART_SOURCE,SfxBoolItem ColHeaders FN_PARAM_1,SfxBoolItem RowHeaders FN_PARAM_2
        # in : Name (string)\nRange (string)\nRowHeaders (bool)
        CR = '<br>'
        split = ''
        params = params.strip(' ,').replace(', ', ',')  #   At least 1 case of ', ' in svx/sdi/svx.sdi line 3592
        if len(params) > 0:
            for p in params.split(','):
                if len(split) > 0:
                    split += CR
                elems = p.split()
                if len(elems) >= 2:
                    split += elems[1]
                    if 'String' in elems[0]:
                        split += ' (string)'
                    elif 'Bool' in elems[0]:
                        split += ' (bool)'
                    elif 'Int16' in elems[0]:
                        split += ' (integer)'
                    elif 'Int32' in elems[0]:
                        split += ' (long)'
                    else:
                        split += ' (' + elems[0].replace('Sfx', '').replace('Svx', '').replace('Item', '').lower() + ')'
        return split

    for filename in SDI_FILES:
        ln = 0
        comment, square, command, param = False, False, False, False
        with open(filename) as fh:
            for line in fh:
                ln += 1
                line = line.replace('  ', ' ').strip()       #   Anomaly met in svx/sdi/svx.sdi
                if line.startswith('//'):
                    pass
                elif comment is False and line.startswith('/*') and not line.endswith('*/'):
                    comment = True
                elif comment is True and line.endswith('*/'):
                    comment = False
                elif comment is False and line.startswith('/*') and line.endswith('*/'):
                    pass
                elif comment is True:
                    pass
                elif square is False and line.startswith('['):
                    square = True
                    mode = ''
                    command = False
                elif square is True and line.endswith(']'):
                    all_commands[command_name]['mode'] = mode
                    square = False
                elif square is True:
                    squaremode = line.strip(',;').split()
                    if len(squaremode) == 3:
                        mode += 'U' if squaremode[0] == 'AutoUpdate' and squaremode[2] == 'TRUE' else ''
                        mode += 'M' if squaremode[0] == 'MenuConfig' and squaremode[2] == 'TRUE' else ''
                        mode += 'T' if squaremode[0] == 'ToolBoxConfig' and squaremode[2] == 'TRUE' else ''
                        mode += 'A' if squaremode[0] == 'AccelConfig' and squaremode[2] == 'TRUE' else ''
                elif comment is False and square is False and command is False and len(line) == 0:
                    pass
                elif command is False:
                    command_name = get_uno(line.split(' ')[1])
                    if command_name not in all_commands:
                        all_commands[command_name] = newcommand(command_name)
                    all_commands[command_name]['sdifile'] = SDI_FILES.index(filename)
                    all_commands[command_name]['sdilinenumber'] = ln
                    all_commands[command_name]['sdioccurs'] += 1
                    if len(all_commands[command_name]['resourceid']) == 0:
                        all_commands[command_name]['resourceid'] = line.split(' ')[2]
                    command = True
                elif command is True and (line == '' or line == '()'):
                    command = False
                elif command is True and (param is True or line.startswith('(')) and line.endswith(')'):
                    if param:
                        params += line.strip(' (),').replace(', ', ',') #   At least 1 case of ", " in svx/sdi/svx.sdi line 8767
                                                                        #   At least 1 case of "( " in sw/sdi/swriter.sdi line 5477
                    else:
                        params = line.strip(' (),').replace(', ', ',')  #   At least 1 case in sw/sdi/swriter.sdi line 7083
                    all_commands[command_name]['arguments'] = SplitArguments(params)
                    command = False
                    param = False
                elif command is True and line.startswith('('):  #   Arguments always on 1 line, except in some cases (cfr.BasicIDEAppear)
                    params = line.strip(' ()').replace(', ', ',')
                    param = True
                elif param is True:
                    params += line


def categorize(all_commands):
    # Clean black listed commands
    for command in BLACKLIST:
        cmd = get_uno(command)
        if cmd in all_commands:
            del all_commands[cmd]
    # Set category based on the file name where the command was found first
    for cmd in all_commands:
        command = all_commands[cmd]
        cxcu, chxx, csdi = '', '', ''
        fxcu = command['xcufile']
        if fxcu > -1:
            cxcu = os.path.basename(XCU_FILES[fxcu]).split('.')[0].replace('Commands', '')
        fhxx = command['hxxfile']
        if fhxx > -1:
            chxx = os.path.basename(HXX_FILES[fhxx]).split('.')[0]
        fsdi = command['sdifile']
        if fsdi > -1:
            csdi = os.path.basename(SDI_FILES[fsdi]).split('.')[0]
        # General rule:
        if len(cxcu) > 0:
            cat = cxcu
        elif len(chxx) > 0:
            cat = chxx
        else:
            cat = csdi
        # Exceptions on general rule
        if cat == 'Generic' and chxx == 'basslots':
            cat = chxx
        command['module'] = MODULES[cat]


def print_output(all_commands):
    def longest(*args):
        # Return the longest string among the arguments
        return max(args, key = len)
    #
    def sources(cmd):
        # Build string identifying the sources
        xcufile, xculinenumber, hxxfile, hxxlinenumber, sdifile, sdilinenumber = 2, 3, 8, 10, 14, 16
        src = ''
        if cmd[xcufile] >= 0:
            src += '[' + REPO + XCU_FILES[cmd[xcufile]].replace(srcdir, '') + '#' + str(cmd[xculinenumber]) + ' XCU]'
        if cmd[sdifile] >= 0:
            src += ' [' + REPO + SDI_FILES[cmd[sdifile]].replace(srcdir, '') + '#' + str(cmd[sdilinenumber]) + ' SDI]'
        if cmd[hxxfile] >= 0:
            file = str(cmd[hxxfile] + 1 + len(XCU_FILES) + len(SDI_FILES))
            src += ' <span title="File (' + file + ') line ' + str(cmd[hxxlinenumber]) + '">[[#hxx' + file + '|HXX]]</span>'
        return src.strip()
    #
    # Sort by category and command name
    commands_list = []
    for cmd in all_commands:
        cmdlist = tuple(all_commands[cmd].values())
        commands_list.append(cmdlist)
    sorted_by_command = sorted(commands_list, key = lambda cmd: cmd[0])
    sorted_by_module = sorted(sorted_by_command, key = lambda cmd: cmd[1])
    #
    # Produce tabular output
    unocommand, module, label, contextlabel, tooltiplabel, arguments, resourceid, numericid, group, mode = 0, 1, 5, 6, 7, 18, 11, 12, 13, 17
    lastmodule = ''
    for cmd in sorted_by_module:
        # Format bottom and header
        if lastmodule != cmd[module]:
            if len(lastmodule) > 0:
                print('\n|-\n|}\n')
                print('</small>')
            lastmodule = cmd[module]
            print('=== %s ===\n' % lastmodule)
            print('<small>')
            print('{| class="wikitable sortable" width="100%"')
            print('|-')
            print('! scope="col" | Dispatch command')
            print('! scope="col" | Description')
            print('! scope="col" | Group')
            print('! scope="col" | Arguments')
            print('! scope="col" | Internal<br>name (value)')
            print('! scope="col" | Mode')
            print('! scope="col" | Source<br>files')
        print('|-\n')
        print('| ' + cmd[unocommand].replace('&amp;', '\n&'))
        print('| ' + longest(cmd[label], cmd[contextlabel], cmd[tooltiplabel]))
        print('| ' + cmd[group])
        print('| ' + cmd[arguments].replace('\\n', '\n'))
        if len(cmd[numericid]) == 0:
            print('| ' + cmd[resourceid])
        else:
            print('| ' + cmd[resourceid] + ' (' + cmd[numericid] + ')')
        print('| ' + cmd[mode])
        print('| ' + sources(cmd))
    print('|-\n|}\n')
    # List the source files
    print('== Source files ==\n')
    fn = 0
    for i in range(len(XCU_FILES)):
        fn += 1
        print(f'({fn}) {REPO}{XCU_FILES[i]}\n'.replace(srcdir, ''))
    print('\n')
    for i in range(len(SDI_FILES)):
        fn += 1
        print(f'({fn}) {REPO}{SDI_FILES[i]}\n'.replace(srcdir, ''))
    print('\n')
    for i in range(len(HXX_FILES)):
        fn += 1
        print(f'<span id="hxx{fn}">({fn}) {HXX_FILES[i]}</span>\n'.replace(builddir, ''))
    print('</small>')


def main():
    all_commands = {}

    analyze_xcu(all_commands)

    analyze_hxx(all_commands)

    analyze_sdi(all_commands)

    categorize(all_commands)

    print_output(all_commands)

if __name__ == '__main__':
    main()
