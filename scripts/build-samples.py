#!/usr/bin/env python3
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# Copyright the Collabora Office contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import os
import re
import sys
import random
import shutil

def usageAndExit():
    message = """usage: {program} template_path dest_path

Expands and fully populates sample files to better exercise
and test Collabora Online.
"""
    print(message.format(program=sys.argv[0]))
    exit(1)

def srcName(srcDir, base, extn):
    return srcDir + '/' + base + '.' + extn

def destName(destDir, base, extn):
    return destDir + '/' + base + '-edit.' + extn

class OutputQueue:
    def __init__(self, fout, lines=5):
        self.fout = fout
        self.maxLines = lines
        self.buf = []
        self.skipPat = None

    def writeBuf(self):
        if len (self.buf) > 0:
            line = self.buf.pop(0)
            self.fout.write(line)
            self.fout.write('\n')

    def writeLine(self, line):
        # allow skipping output until a regex is hit
        if self.skipPat:
            if not self.skipPat.match(line):
                return # skip
            else:
                self.skipPat = None
                return # last one

        self.buf.append(line)
        if len (self.buf) > self.maxLines:
            self.writeBuf()

    def flush(self):
        for i in range(0, len(self.buf)):
            self.writeBuf();

    def skipTo(self, pat):
        self.skipPat = pat

    def dropLines(self, count):
        self.buf = self.buf[:-count]

def substitute(queue, var):
    print('substitute ' + var)
    if var == 'RAW_DATA':
        queue.dropLines(2) # table-cell, table-row - and current line
        firstName = [ 'Dulce', 'Mara', 'Philip', 'Kathleen', \
                      'Nereida', 'Gaston', 'Etta', 'Earlean', 'Vincenza', \
                      'Fallon', 'Arcelia', 'Franklyn', 'Sherron', 'Marcel', \
                      'Kina', 'Shavonne', 'Shavon', 'Lauralee', 'Loreta', \
                      'Teresa', 'Belinda', 'Holly', 'Many', 'Libbie', 'Lester', \
                      'Marvel', 'Angelyn', 'Francesca', 'Garth', 'Carla', \
                      'Veta', 'Stasia', 'Jona', 'Judie', 'Dewitt', 'Nena', \
                      'Kelsie', 'Sau', 'Shanice', 'Chase', 'Tommie', 'Dorcas', \
                      'Angel', 'Willodean', 'Weston', 'Roma', 'Felisa', 'Demetria', \
                      'Jeromy', 'Rasheeda' ]
        secondName = [ 'Abril', 'Hashimoto', 'Gent', 'Hanner', 'Magwood', 'Brumm', \
                       'Hurn', 'Melgar', 'Weiland', 'Winward', 'Bouska', 'Unknown', \
                       'Ascencio', 'Zabriskie', 'Hazelton', 'Pia', 'Benito', 'Perrine', \
                       'Curren', 'Strawn', 'Partain', 'Eudy', 'Cuccia', 'Dalby', \
                       'Prothro', 'Hail', 'Vong', 'Beaudreau', 'Gangi', 'Trumbull', \
                       'Muntz', 'Becker', 'Grindle', 'Claywell', 'Borger', 'Hacker', \
                       'Wachtel', 'Pfau', 'Mccrystal', 'Karner', 'Underdahl', 'Darity', \
                       'Sanor', 'Harn', 'Martina', 'Lafollette', 'Cail', 'Abbey', \
                       'Danz', 'Alkire' ]
        gender = [ 'Female', 'Female', 'Male', 'Female', 'Female', 'Male', 'Female', \
                   'Female', 'Female', 'Female', 'Female', 'Male', 'Female', 'Male', \
                   'Female', 'Female', 'Female', 'Female', 'Female', 'Female', 'Female', \
                   'Female', 'Female', 'Female', 'Male', 'Female', 'Female', 'Female', \
                   'Male', 'Female', 'Female', 'Female', 'Female', 'Female', 'Male', \
                   'Female', 'Female', 'Female', 'Female', 'Male', 'Male', 'Female', \
                   'Male', 'Female', 'Male', 'Female', 'Female', 'Female', 'Male', \
                   'Female' ]
        geo = [ 'United States', 'Great Britain', 'Germany', 'France' ]

        rng = random.Random()
        rng.seed(42)

        for i in range(1,5000):
            queue.writeLine('<table:table-row table:style-name="ro2">')

            queue.writeLine('<table:table-cell office:value-type="float" office:value="' + str(i) + '" calcext:value-type="float">')
            queue.writeLine('    <text:p>' + str(i) + '</text:p>')
            queue.writeLine('</table:table-cell>')
            queue.writeLine('<table:table-cell office:value-type="string" calcext:value-type="string">')
            queue.writeLine('<text:p>' + firstName[i % len(firstName)] + '</text:p>')
            queue.writeLine('</table:table-cell>')
            queue.writeLine('<table:table-cell office:value-type="string" calcext:value-type="string">')
            queue.writeLine('<text:p>' + secondName[i % len(secondName)] + '</text:p>')
            queue.writeLine('</table:table-cell>')
            queue.writeLine('<table:table-cell office:value-type="string" calcext:value-type="string">')
            queue.writeLine('<text:p>' + gender[i % len(gender)] + '</text:p>')
            queue.writeLine('</table:table-cell>')
            queue.writeLine('<table:table-cell office:value-type="string" calcext:value-type="string">')
            queue.writeLine('<text:p>' + geo[i % len(geo)] + '</text:p>')
            queue.writeLine('</table:table-cell>')

            v = str(rng.randint(16, 96))
            queue.writeLine('<table:table-cell office:value-type="float" office:value="' + v + '" calcext:value-type="float">')
            queue.writeLine('<text:p>' + v + '</text:p>')
            queue.writeLine('</table:table-cell>')

            # date
            dt = str(rng.randint(2000, 2025)) + '-' + str(rng.randint(10, 12)) + '-' + str(rng.randint(10, 28))
            queue.writeLine('<table:table-cell office:value-type="string" calcext:value-type="string">')
            queue.writeLine('<text:p>' + dt + '</text:p>')
            queue.writeLine('</table:table-cell>')

            # id - sparse
            if (rng.randint(0,100) > 10):
                v = str(rng.randint(-100000, 100000)/100.0)
                queue.writeLine('<table:table-cell office:value-type="float" office:value="' + v +
                                '" calcext:value-type="float">')
                queue.writeLine('<text:p>' + v + '</text:p>')
                queue.writeLine('</table:table-cell>')
            else: # empty
                queue.writeLine('<table:table-cell/>')

            # Percentage
            v = str(rng.randint(-2000, 5000)/10.0)
            queue.writeLine('<table:table-cell office:value-type="percentage" office:value="' + v +
                            '" calcext:value-type="percentage">')
            queue.writeLine('<text:p>' + v + '</text:p>')
            queue.writeLine('</table:table-cell>')

            # USD
            v = rng.randint(-100000, 100000)/100.0
            queue.writeLine('<table:table-cell office:value-type="currency" office:currency="USD" office:value="' + str(v) +
                            '" calcext:value-type="currency">')
            queue.writeLine('<text:p>$' + str(v) + '</text:p>')
            queue.writeLine('</table:table-cell>')

            # GBP
            cellRef = 'J' + str(i + 2)
            queue.writeLine('<table:table-cell table:formula="of:=[.' + cellRef + ']*0.8" ' +
                            'office:value-type="currency" office:currency="GBP" office:value="' +
                            str(v*0.8) + '" calcext:value-type="currency">')
            queue.writeLine('<text:p>£' + str(v*0.8) + '</text:p>')
            queue.writeLine('</table:table-cell>')

            # EUR
            cellRef = 'K' + str(i + 2)
            queue.writeLine('<table:table-cell table:formula="of:=[.' + cellRef + ']*0.8" ' +
                            'office:value-type="percentage" office:value="' +
                            str(v*0.8) + '" calcext:value-type="percentage">')
            queue.writeLine('<text:p>' + str(v*0.8*100) + '</text:p>')
            queue.writeLine('</table:table-cell>')

            cellRef = 'J' + str(i + 2)
            queue.writeLine('<table:table-cell table:formula="of:=[.' + cellRef + ']*0.93" ' +
                            'office:value-type="currency" office:currency="EUR" office:value="' +
                            str(v*0.93) + '" calcext:value-type="currency">')
            queue.writeLine('<text:p>$' + str(v*0.93) + '</text:p>')
            queue.writeLine('</table:table-cell>')

            # 4k screens need more data
            for i in range(0,32):
                v = str(rng.uniform(0, 1000))
                queue.writeLine('<table:table-cell office:value-type="float" office:value="' + v + '" calcext:value-type="float">')
                queue.writeLine('<text:p>' + v + '</text:p>')
                queue.writeLine('</table:table-cell>')

            queue.writeLine('<table:table-cell table:number-columns-repeated="979"/>')

            queue.writeLine('</table:table-row>')

        queue.skipTo(re.compile('.*/table:table-row.*'))

# FIXME: should generate and insert into an fods template
# lots of calc content for testing scaling without lots of
# data in git.
def generateCalc(srcDir, destDir, base, extn):
    dest = destName(srcDir, base, extn)
    sys.stdout.write('Calc sheet generation into ' + dest + '\n')
    fin = open(srcName(srcDir, base, extn), 'r', encoding='utf-8')
    fout = open(dest, 'w', encoding='utf-8')

    pattern = re.compile('.*%([A-Z_]+)%.*')
    queue = OutputQueue(fout);
    for line in fin:
        var = pattern.match(line)
        if var:
            substitute(queue, var.group(1))
        queue.writeLine(line)

    queue.flush()
    fout.close()
    fin.close()

def generateWriter(srcDir, destDir, base, extn):
    fin = open(srcName(srcDir, base, extn), 'r', encoding='utf-8')
    fout = open(destName(srcDir, base, extn), 'w', encoding='utf-8')

    # The template has a single paragraph: paste it enough times so that we get a document of ~300
    # pages.
    pattern = re.compile('.*He heard quiet steps behind him.*')
    for line in fin:
        if pattern.match(line):
            for _ in range(825):
                fout.write(line)
        else:
            fout.write(line)

    fout.close()
    fin.close()

def copyFile(srcDir, destDir, base, extn):
    sys.stdout.write('Copy ' + base + ' to ' + destDir + '\n')
    shutil.copy2(srcName(srcDir, base, extn), destName(destDir, base, extn))

if __name__ == "__main__":
    if len(sys.argv) < 2:
        usageAndExit()

    src = sys.argv[1];
    dest = sys.argv[2];

    for file in os.listdir(src):
        if file[0] == '.':
            continue;

        bits = file.rsplit('.', 1)
        base = bits[0]
        extn = bits[1]

        if base.endswith('-edit'):
            continue # srcdir=builddir

        if file.endswith('calc.fods'):
            generateCalc(src, dest, base, extn)
        elif file.endswith('writer-large.fodt'):
            generateWriter(src, dest, base, extn)
        else:
            copyFile(src, dest, base, extn)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
