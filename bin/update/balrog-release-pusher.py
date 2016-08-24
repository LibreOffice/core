#!/usr/bin/env python
try:
    import simplejson as json
except ImportError:
    import json

import os
from os import path
import re
import logging
import sys

# Use explicit version of python-requests
sys.path.insert(0, path.join(path.dirname(__file__),
                             "../../lib/python/vendor/requests-2.7.0"))

from balrog.submitter.cli import ReleaseCreatorV3, ReleaseCreatorV4, \
    ReleasePusher
from release.info import readReleaseConfig
from util.retry import retry

REQUIRED_CONFIG = ('appVersion', 'productName', 'version', 'enUSPlatforms',
    'baseTag', 'updateChannels', 'buildNumber', 'partialUpdates',
    'ftpServer', 'bouncerServer')

def validate(options):
    err = False
    config = {}

    if not path.exists(path.join('buildbot-configs', options.release_config)):
        print "%s does not exist!" % options.release_config
        sys.exit(1)

    config = readReleaseConfig(path.join('buildbot-configs',
                                         options.release_config))
    for key in REQUIRED_CONFIG:
        if key not in config:
            err = True
            print "Required item missing in config: %s" % key

    if err:
        sys.exit(1)
    return config

if __name__ == '__main__':

    from optparse import OptionParser
    parser = OptionParser()
    parser.add_option("-p", "--build-properties", dest="build_properties")
    parser.add_option("-b", "--buildbot-configs", dest="buildbot_configs",
                      help="The place to clone buildbot-configs from"))
    parser.add_option("-r", "--release-config", dest="release_config")
    parser.add_option("-a", "--api-root", dest="api_root")
    parser.add_option("-c", "--credentials-file", dest="credentials_file")
    parser.add_option("-s", "--schema", dest="schema_version",
                      help="blob schema version", type="int", default=4)
    parser.add_option("-u", "--username", dest="username")
    parser.add_option("-C", "--release-channel", dest="release_channel")
    parser.add_option("-v", "--verbose", dest="verbose", action="store_true")
    options, args = parser.parse_args()

    logging_level = logging.INFO
    if options.verbose:
        logging_level = logging.DEBUG
    logging.basicConfig(stream=sys.stdout, level=logging_level,
                        format="%(message)s")

    for opt in ('build_properties', 'release_config', 'api_root', 'credentials_file', 'buildbot_configs', 'username', 'release_channel'):
        if not getattr(options, opt):
            print >>sys.stderr, "Required option %s not present" % opt
            sys.exit(1)

    if options.schema_version not in (3,4):
        parser.error("Only schema_versions 3 & 4 supported.")

    release_channel = options.release_channel
    properties = json.load(open(options.build_properties))['properties']
    releaseTag = properties['script_repo_revision']
    hashType = properties['hashType']
    retry(mercurial, args=(options.buildbot_configs, 'buildbot-configs'), kwargs=dict(revision=releaseTag))
    release_config = validate(options)
    channelInfo = release_config["updateChannels"][release_channel]

    credentials = {}
    execfile(options.credentials_file, credentials)
    auth = (options.username, credentials['balrog_credentials'][options.username])
    updateChannels = [
        release_channel,
        channelInfo['localTestChannel'],
        channelInfo['cdnTestChannel']
    ]

    if options.schema_version == 3:
        creator = ReleaseCreatorV3(options.api_root, auth)
    else:
        creator= ReleaseCreatorV4(options.api_root, auth)
    partials = {}
    for v in release_config['partialUpdates']:
        if re.match(channelInfo.get("versionRegex", "^.*$"), v):
            partials[v] = release_config["partialUpdates"][v]

    creator.run(release_config['appVersion'], release_config['productName'].capitalize(),
                release_config['version'], release_config['buildNumber'],
                updateChannels, release_config['ftpServer'],
                release_config['bouncerServer'], release_config['enUSPlatforms'],
                hashType, openURL=release_config.get('openURL'),
                partialUpdates=partials,
                requiresMirrors=channelInfo.get("requiresMirrors", True))

    testChannelRuleIds = []
    for c in channelInfo["testChannels"].values():
        testChannelRuleIds.append(c["ruleId"])
    pusher = ReleasePusher(options.api_root, auth)
    pusher.run(release_config['productName'].capitalize(), release_config['version'],
               release_config['buildNumber'], testChannelRuleIds)
