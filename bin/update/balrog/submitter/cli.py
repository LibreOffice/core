try:
    import simplejson as json
except ImportError:
    import json

from release.info import getProductDetails
# from release.paths import makeCandidatesDir
from release.platforms import buildbot2updatePlatforms, buildbot2bouncer, \
  buildbot2ftp
from release.versions import getPrettyVersion
from balrog.submitter.api import Release, SingleLocale, Rule
from balrog.submitter.updates import merge_partial_updates
from util.algorithms import recursive_update
# from util.retry import retry
import logging
from requests.exceptions import HTTPError

log = logging.getLogger(__name__)


def get_nightly_blob_name(productName, branch, build_type, suffix, dummy=False):
    if dummy:
        branch = '%s-dummy' % branch
    return '%s-%s-%s-%s' % (productName, branch, build_type, suffix)


def get_release_blob_name(productName, version, build_number, dummy=False):
    name = '%s-%s-build%s' % (productName, version, build_number)
    if dummy:
        name += '-dummy'
    return name


class ReleaseCreatorBase(object):
    def __init__(self, api_root, auth, dummy=False):
        self.api_root = api_root
        self.auth = auth
        self.dummy = dummy

    def generate_data(self, appVersion, productName, version, buildNumber,
                      updateChannels, ftpServer, bouncerServer,
                      enUSPlatforms, schemaVersion, openURL=None,
                      **updateKwargs):
        assert schemaVersion in (3, 4), 'Unhandled schema version %s' % schemaVersion
        data = {
            'detailsUrl': getProductDetails(productName.lower(), appVersion),
            'platforms': {},
            'fileUrls': {},
            'appVersion': appVersion,
            'platformVersion': appVersion,
            'displayVersion': getPrettyVersion(version)
        }

        actions = []
        if openURL:
            actions.append("showURL")
            data["openURL"] = openURL

        if actions:
            data["actions"] = " ".join(actions)

        fileUrls = self._getFileUrls(productName, version, buildNumber,
                                     updateChannels, ftpServer,
                                     bouncerServer, **updateKwargs)
        if fileUrls:
            data.update(fileUrls)

        updateData = self._get_update_data(productName, version, **updateKwargs)
        if updateData:
            data.update(updateData)

        for platform in enUSPlatforms:
            updatePlatforms = buildbot2updatePlatforms(platform)
            bouncerPlatform = buildbot2bouncer(platform)
            ftpPlatform = buildbot2ftp(platform)
            data['platforms'][updatePlatforms[0]] = {
                'OS_BOUNCER': bouncerPlatform,
                'OS_FTP': ftpPlatform
            }
            for aliasedPlatform in updatePlatforms[1:]:
                data['platforms'][aliasedPlatform] = {
                    'alias': updatePlatforms[0]
                }

        return data

    def run(self, appVersion, productName, version, buildNumber,
            updateChannels, ftpServer, bouncerServer,
            enUSPlatforms, hashFunction, schemaVersion, openURL=None,
            **updateKwargs):
        data = self.generate_data(appVersion, productName, version,
                                  buildNumber, updateChannels,
                                  ftpServer, bouncerServer, enUSPlatforms,
                                  schemaVersion, openURL, **updateKwargs)
        name = get_release_blob_name(productName, version, buildNumber,
                                     self.dummy)
        api = Release(name=name, auth=self.auth, api_root=self.api_root)
        try:
            current_data, data_version = api.get_data()
        except HTTPError, e:
            if e.response.status_code == 404:
                log.warning("Release blob doesn't exist, using empty data...")
                current_data, data_version = {}, None
            else:
                raise

        data = recursive_update(current_data, data)
        api.update_release(product=productName,
                           hashFunction=hashFunction,
                           releaseData=json.dumps(data),
                           schemaVersion=schemaVersion,
                           data_version=data_version)


class ReleaseCreatorV3(ReleaseCreatorBase):
    def run(self, *args, **kwargs):
        return ReleaseCreatorBase.run(self, *args, schemaVersion=3, **kwargs)

    def _getFileUrls(self, productName, version, buildNumber, updateChannels,
                     ftpServer, bouncerServer, partialUpdates):
        data = {}

        for channel in updateChannels:
            if channel in ('betatest', 'esrtest') or "localtest" in channel:
                # TODO: moggi: what does this do?
                # dir_ = makeCandidatesDir(productName.lower(), version,
                #                          buildNumber, server=ftpServer, protocol='http')
                # data["fileUrls"][channel] = '%supdate/%%OS_FTP%%/%%LOCALE%%/%%FILENAME%%' % dir_
                pass
            else:
                url = 'http://%s/?product=%%PRODUCT%%&os=%%OS_BOUNCER%%&lang=%%LOCALE%%' % bouncerServer
                data["fileUrls"][channel] = url

        return data

    def _get_update_data(self, productName, version, partialUpdates):
        data = {
            "ftpFilenames": {
                "completes": {
                    "*": "%s-%s.complete.mar" % (productName.lower(), version),
                }
            },
            "bouncerProducts": {
                "completes": {
                    "*": "%s-%s-complete" % (productName.lower(), version),
                }
            }
        }

        if partialUpdates:
            data["ftpFilenames"]["partials"] = {}
            data["bouncerProducts"]["partials"] = {}
            for previousVersion, previousInfo in partialUpdates.iteritems():
                from_ = get_release_blob_name(productName, previousVersion,
                                              previousInfo["buildNumber"],
                                              self.dummy)
                filename = "%s-%s-%s.partial.mar" % (productName.lower(), previousVersion, version)
                bouncerProduct = "%s-%s-partial-%s" % (productName.lower(), version, previousVersion)
                data["ftpFilenames"]["partials"][from_] = filename
                data["bouncerProducts"]["partials"][from_] = bouncerProduct

        return data


class ReleaseCreatorV4(ReleaseCreatorBase):
    def run(self, *args, **kwargs):
        return ReleaseCreatorBase.run(self, *args, schemaVersion=4, **kwargs)

    # Replaced by _get_fileUrls
    def _get_update_data(self, *args, **kwargs):
        return None

    def _getFileUrls(self, productName, version, buildNumber, updateChannels,
                     ftpServer, bouncerServer, partialUpdates,
                     requiresMirrors=True):
        data = {"fileUrls": {}}

        # "*" is for the default set of fileUrls, which generally points at
        # bouncer. It's helpful to have this to reduce duplication between
        # the live channel and the cdntest channel (which eliminates the
        # possibility that those two channels serve different contents).
        uniqueChannels = ["*"]
        for c in updateChannels:
            # localtest channels are different than the default because they
            # point directly at FTP rather than Bouncer.
            if "localtest" in c:
                uniqueChannels.append(c)
            # beta and beta-cdntest are special, but only if requiresMirrors is
            # set to False. This is typically used when generating beta channel
            # updates as part of RC builds, which get shipped prior to the
            # release being pushed to mirrors. This is a bit of a hack.
            if not requiresMirrors and c in ("beta", "beta-cdntest",
                                             "beta-dev", "beta-dev-cdntest"):
                uniqueChannels.append(c)

        for channel in uniqueChannels:
            data["fileUrls"][channel] = {
                "completes": {}
            }
            if "localtest" in channel:
                pass
                # dir_ = makeCandidatesDir(productName.lower(), version,
                #                          buildNumber, server=ftpServer,
                #                          protocol='http')
                # filename = "%s-%s.complete.mar" % (productName.lower(), version)
                # data["fileUrls"][channel]["completes"]["*"] = "%supdate/%%OS_FTP%%/%%LOCALE%%/%s" % (dir_, filename)
            else:
                # See comment above about these channels for explanation.
                if not requiresMirrors and channel in ("beta", "beta-cdntest", "beta-dev", "beta-dev-cdntest"):
                    bouncerProduct = "%s-%sbuild%s-complete" % (productName.lower(), version, buildNumber)
                else:
                    if productName.lower() == "fennec":
                        bouncerProduct = "%s-%s" % (productName.lower(), version)
                    else:
                        bouncerProduct = "%s-%s-complete" % (productName.lower(), version)
                url = 'http://%s/?product=%s&os=%%OS_BOUNCER%%&lang=%%LOCALE%%' % (bouncerServer, bouncerProduct)
                data["fileUrls"][channel]["completes"]["*"] = url

        if not partialUpdates:
            return data

        for channel in uniqueChannels:
            data["fileUrls"][channel]["partials"] = {}
            for previousVersion, previousInfo in partialUpdates.iteritems():
                from_ = get_release_blob_name(productName, previousVersion,
                                                previousInfo["buildNumber"],
                                                self.dummy)
                if "localtest" in channel:
                    pass
                    # dir_ = makeCandidatesDir(productName.lower(), version,
                    #                         buildNumber, server=ftpServer,
                    #                         protocol='http')
                    # filename = "%s-%s-%s.partial.mar" % (productName.lower(), previousVersion, version)
                    # data["fileUrls"][channel]["partials"][from_] = "%supdate/%%OS_FTP%%/%%LOCALE%%/%s" % (dir_, filename)
                else:
                    # See comment above about these channels for explanation.
                    if not requiresMirrors and channel in ("beta", "beta-cdntest", "beta-dev", "beta-dev-cdntest"):
                        bouncerProduct = "%s-%sbuild%s-partial-%sbuild%s" % (productName.lower(), version, buildNumber, previousVersion, previousInfo["buildNumber"])
                    else:
                        bouncerProduct = "%s-%s-partial-%s" % (productName.lower(), version, previousVersion)
                    url = 'http://%s/?product=%s&os=%%OS_BOUNCER%%&lang=%%LOCALE%%' % (bouncerServer, bouncerProduct)
                    data["fileUrls"][channel]["partials"][from_] = url

        return data


class NightlySubmitterBase(object):
    build_type = 'nightly'

    def __init__(self, api_root, auth, dummy=False, url_replacements=None):
        self.api_root = api_root
        self.auth = auth
        self.dummy = dummy
        self.url_replacements = url_replacements

    def _replace_canocical_url(self, url):
        if self.url_replacements:
            for string_from, string_to in self.url_replacements:
                if string_from in url:
                    new_url = url.replace(string_from, string_to)
                    log.warning("Replacing %s with %s", url, new_url)
                    return new_url

        return url

    def run(self, platform, buildID, productName, branch, appVersion, locale,
            hashFunction, extVersion, schemaVersion, **updateKwargs):
        assert schemaVersion in (3,4), 'Unhandled schema version %s' % schemaVersion
        targets = buildbot2updatePlatforms(platform)
        build_target = targets[0]
        alias = None
        if len(targets) > 1:
            alias = targets[1:]
        data = {
            'buildID': buildID,
            'appVersion': appVersion,
            'platformVersion': extVersion,
            'displayVersion': appVersion,
        }

        data.update(self._get_update_data(productName, branch, **updateKwargs))

        if platform == 'android-api-9':
            # Bug 1080749 - a hack to support api-9 and api-10+ split builds.
            # Like 1055305, this is a hack to support two builds with same build target that
            # require differed't release blobs and rules
            build_type = 'api-9-%s' % self.build_type
        else:
            build_type = self.build_type

        name = get_nightly_blob_name(productName, branch, build_type, buildID,
                                     self.dummy)
        api = SingleLocale(name=name, build_target=build_target, locale=locale,
                           auth=self.auth, api_root=self.api_root)

        # wrap operations into "atomic" functions that can be retried
        def update_dated():
            current_data, data_version = api.get_data()
            # If the  partials are already a subset of the blob and the
            # complete MAR is the same, skip the submission
            skip_submission = bool(
                current_data and
                current_data.get("completes") == data.get("completes") and
                all(p in current_data.get("partials", [])
                    for p in data.get("partials", [])))
            if skip_submission:
                log.warn("Dated data didn't change, skipping update")
                return
            # explicitly pass data version
            api.update_build(
                product=productName,
                hashFunction=hashFunction,
                buildData=json.dumps(merge_partial_updates(current_data,
                                                           data)),
                alias=json.dumps(alias),
                schemaVersion=schemaVersion, data_version=data_version)

        # TODO: moggi: enable retry again
        # retry(update_dated, sleeptime=10)
        update_dated()

        latest = SingleLocale(
            api_root=self.api_root, auth=self.auth,
            name=get_nightly_blob_name(productName, branch, build_type,
                                       'latest', self.dummy),
            build_target=build_target, locale=locale)

        def update_latest():
            # copy everything over using target release's data version
            latest_data, latest_data_version = latest.get_data()
            source_data, _ = api.get_data()
            if source_data == latest_data:
                log.warn("Latest data didn't change, skipping update")
                return
            latest.update_build(
                product=productName,
                hashFunction=hashFunction, buildData=json.dumps(source_data),
                alias=json.dumps(alias), schemaVersion=schemaVersion,
                data_version=latest_data_version)

        # TODO: moggi: enable retry again
        # retry(update_latest, sleeptime=10)
        update_latest()


class MultipleUpdatesNightlyMixin(object):

    def _get_update_data(self, productName, branch, completeInfo=None,
                         partialInfo=None):
        data = {}

        if completeInfo:
            data["completes"] = []
            for info in completeInfo:
                if "from_buildid" in info:
                    from_ = get_nightly_blob_name(productName, branch,
                                                  self.build_type,
                                                  info["from_buildid"],
                                                  self.dummy)
                else:
                    from_ = "*"
                data["completes"].append({
                    "from": from_,
                    "filesize": info["size"],
                    "hashValue": info["hash"],
                    "fileUrl": self._replace_canocical_url(info["url"]),
                })
        if partialInfo:
            data["partials"] = []
            for info in partialInfo:
                data["partials"].append({
                    "from": get_nightly_blob_name(productName, branch,
                                                  self.build_type,
                                                  info["from_buildid"],
                                                  self.dummy),
                    "filesize": info["size"],
                    "hashValue": info["hash"],
                    "fileUrl": self._replace_canocical_url(info["url"]),
                })

        return data


class NightlySubmitterV3(NightlySubmitterBase, MultipleUpdatesNightlyMixin):
    def run(self, *args, **kwargs):
        return NightlySubmitterBase.run(self, *args, schemaVersion=3, **kwargs)


class NightlySubmitterV4(NightlySubmitterBase, MultipleUpdatesNightlyMixin):
    def run(self, *args, **kwargs):
        return NightlySubmitterBase.run(self, *args, schemaVersion=4, **kwargs)


class ReleaseSubmitterBase(object):
    def __init__(self, api_root, auth, dummy=False):
        self.api_root = api_root
        self.auth = auth
        self.dummy = dummy

    def run(self, platform, productName, appVersion, version, build_number, locale,
            hashFunction, extVersion, buildID, schemaVersion, **updateKwargs):
        assert schemaVersion in (3, 4), 'Unhandled schema version %s' % schemaVersion
        targets = buildbot2updatePlatforms(platform)
        # Some platforms may have alias', but those are set-up elsewhere
        # for release blobs.
        build_target = targets[0]

        name = get_release_blob_name(productName, version, build_number,
                                     self.dummy)
        data = {
            'buildID': buildID,
            'appVersion': appVersion,
            'platformVersion': extVersion,
            'displayVersion': getPrettyVersion(version)
        }

        data.update(self._get_update_data(productName, version, build_number,
                                          **updateKwargs))

        api = SingleLocale(name=name, build_target=build_target, locale=locale,
                           auth=self.auth, api_root=self.api_root)
        schemaVersion = json.dumps(schemaVersion)
        current_data, data_version = api.get_data()
        api.update_build(
            data_version=data_version,
            product=productName, hashFunction=hashFunction,
            buildData=json.dumps(merge_partial_updates(current_data, data)),
            schemaVersion=schemaVersion)


class MultipleUpdatesReleaseMixin(object):
    def _get_update_data(self, productName, version, build_number,
                         completeInfo=None, partialInfo=None):
        data = {}

        if completeInfo:
            data["completes"] = []
            for info in completeInfo:
                if "previousVersion" in info:
                    from_ = get_release_blob_name(productName, version,
                                                  build_number, self.dummy)
                else:
                    from_ = "*"
                data["completes"].append({
                    "from": from_,
                    "filesize": info["size"],
                    "hashValue": info["hash"],
                })
        if partialInfo:
            data["partials"] = []
            for info in partialInfo:
                data["partials"].append({
                    "from": get_release_blob_name(productName,
                                                  info["previousVersion"],
                                                  info["previousBuildNumber"] ,
                                                  self.dummy),
                    "filesize": info["size"],
                    "hashValue": info["hash"],
                })

        return data


class ReleaseSubmitterV3(ReleaseSubmitterBase, MultipleUpdatesReleaseMixin):
    def run(self, *args, **kwargs):
        return ReleaseSubmitterBase.run(self, *args, schemaVersion=3, **kwargs)


class ReleaseSubmitterV4(ReleaseSubmitterBase, MultipleUpdatesReleaseMixin):
    def run(self, *args, **kwargs):
        return ReleaseSubmitterBase.run(self, *args, schemaVersion=4, **kwargs)


class ReleasePusher(object):
    def __init__(self, api_root, auth, dummy=False):
        self.api_root = api_root
        self.auth = auth
        self.dummy = dummy

    def run(self, productName, version, build_number, rule_ids):
        name = get_release_blob_name(productName, version, build_number,
                                     self.dummy)
        for rule_id in rule_ids:
            Rule(api_root=self.api_root, auth=self.auth, rule_id=rule_id
                 ).update_rule(mapping=name)


class BlobTweaker(object):
    def __init__(self, api_root, auth):
        self.api_root = api_root
        self.auth = auth

    def run(self, name, data):
        api = Release(name=name, auth=self.auth, api_root=self.api_root)
        current_data, data_version = api.get_data()
        data = recursive_update(current_data, data)
        api.update_release(
            product=name.split('-')[0],
            hashFunction=data['hashFunction'], releaseData=json.dumps(data),
            data_version=data_version,
            schemaVersion=current_data['schema_version'])

