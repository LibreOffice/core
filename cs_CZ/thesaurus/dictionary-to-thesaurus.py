#!/usr/bin/env python3
# coding=utf-8
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# This utility translates a normal dictionary (in this case English/Czech)
# into a thesaurus for one of the languages (in this case Czech).
#
# Based on idea of Zdenek Zabokrtsky <zabokrtsky@ufal.mff.cuni.cz>, big
# thanks! :-)

import os
import re
import sys


def usage():
    message = """Usage: {program} en-cs.txt blacklist.txt

  en-cs.txt:     Dictionary data from https://www.svobodneslovniky.cz/
  blacklist.txt: List of words that should be ignored when generating
"""
    print(
        message.format(program=os.path.basename(sys.argv[0])),
        file=sys.stderr)


def classify(typ):
    if typ == '':
        return ''
    elif typ == 'adj':
        return '(příd. jm.)'
    elif typ == 'adv':
        return '(přísl.)'
    elif typ == 'n':
        return '(podst. jm.)'
    elif typ == 'v':
        return '(slov.)'

    return ''


def parse(filename, blacklistname):
    blacklist = {}

    with open(blacklistname, "r") as fp:
        for line in fp:
            if (line == ''):
                continue
            elif (line[0] == '#'):
                continue
            else:
                blacklist[line.strip(' \n')] = 1

    synonyms = {}
    meanings = {}
    classification = {}

    match_ignore = re.compile(r'(\[neprav\.\]|\[vulg\.\])')
    match_cleanup = re.compile(r'(\[.*\]|\*|:.*)')

    with open(filename, "r") as fp:
        for line in fp:
            if (line == ''):
                continue
            elif (line[0] == '#'):
                continue
            else:
                terms = line.split('\t')
                if (terms[0] == '' or len(terms) < 2):
                    continue

                index = terms[0].strip()
                if (index == ''):
                    continue

                word = terms[1].strip()
                if (word != '' and word[0] == '"' and word[len(word)-1] == '"'):
                    word = word.strip('" ')

                if (word == ''):
                    continue

                if (index + '\t' + word in blacklist or
                        index in blacklist or
                        index + '\t' in blacklist or
                        '\t' + word in blacklist):
                    continue

                typ = ''
                if (len(terms) >= 2):
                    typ = terms[2]

                    # ignore non-translations
                    if match_ignore.search(typ) is not None:
                        continue

                    typ = match_cleanup.sub('', typ)
                    typ = typ.strip()

                typ = classify(typ)

                if index in synonyms:
                    synonyms[index].append((word, typ))
                else:
                    synonyms[index] = [(word, typ)]

                if word in meanings:
                    meanings[word].append(index)
                else:
                    meanings[word] = [index]

                if typ != '':
                    if word in classification:
                        if typ not in classification[word]:
                            classification[word].append(typ)
                    else:
                        classification[word] = [typ]

    return (synonyms, meanings, classification)


def buildThesaurus(synonyms, meanings, classification):
    # for every word:
    #   find all the indexes, and then again map the indexes to words - these are the synonyms
    for word in sorted(meanings.keys()):
        # we assume that various indexes (english words here) are various
        # meanings; not generally true, but...
        indexes = meanings[word]

        # only limit the words if the type is unambiguous
        typ = ''
        if word in classification and len(classification[word]) == 1:
            typ = classification[word][0]

        # we want to output each word just once
        used_this_round = [word]

        output_lines = []
        for index in indexes:
            syns = synonyms[index]

            # collect types first
            types = []
            for (w, t) in syns:
                if t not in types:
                    types.append(t)

            # build the various thesaurus lines
            line = {}
            for syn in syns:
                (w, t) = syn

                if typ != '' and t != '' and typ != t:
                    continue

                if w not in used_this_round:
                    if t in line:
                        line[t] += '|' + w
                    else:
                        line[t] = '|' + w
                    used_this_round.append(w)

            if len(line) != 0:
                for t in types:
                    if t in line:
                        output_lines.append((t, line[t]))

        if len(output_lines) > 0:
            print(word + '|' + str(len(output_lines)))

            # those with existing classification are probably a better fit,
            # put them to the front (even if we don't output the
            # classification in the end)
            for i in [0, 1]:
                for (t, line) in output_lines:
                    # first pass only non-empty, 2nd pass only empty
                    if (i == 0 and t != '') or (i == 1 and t == ''):
                        if typ == '':
                            print(t + line)
                        else:
                            print(line)


def main(args):
    if (len(args) != 3):
        usage()
        sys.exit(1)

    (synonyms, meanings, classification) = parse(args[1], args[2])

    print("UTF-8")
    buildThesaurus(synonyms, meanings, classification)


if __name__ == "__main__":
    main(sys.argv)

# vim:set shiftwidth=4 softtabstop=4 expandtab:
