#!/usr/bin/env python

import json, re, subprocess, sys, urllib3

http = urllib3.PoolManager()

# TDF implementer notes pages for LibreOffice
wiki_pages = [
    'https://wiki.documentfoundation.org/api.php?action=parse&format=json&page=Development/ODF_Implementer_Notes/List_of_LibreOffice_ODF_Extensions&prop=wikitext',
    'https://wiki.documentfoundation.org/api.php?action=parse&format=json&page=Development/ODF_Implementer_Notes/List_of_LibreOffice_OpenFormula_Extensions&prop=wikitext']

# get all commit hashes mentioned in implementer notes
wiki_commit_hashes = {}
query = re.compile(r'\{\{commit\|(\w+)\|\w*\|\w*\}\}', re.IGNORECASE)
for page in wiki_pages:
    r = http.request('GET', page)
    data = json.loads(r.data.decode('utf-8'))
    for line in data['parse']['wikitext']['*'].split('\n'):
        for res in query.finditer(line):
            wiki_commit_hashes[res.group(1)] = ''

# get all commits that change core/schema/* - and are _not_ mentioned
# in the wiki page
# Cut-off is May 18th 2020, when Michael Stahl had finished cleaning this up
for commit in subprocess.check_output(
        ['git', '--no-pager', '-C', sys.path[0]+'/..', 'log',
         '--since=2020-05-18', '--format=%H', '--', 'schema/'],
        stderr=subprocess.STDOUT).decode("utf-8").split("\n"):
    if commit != '' and commit not in wiki_commit_hashes:
        print('missing commit: %s' % commit)

