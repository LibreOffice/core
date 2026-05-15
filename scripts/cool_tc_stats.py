#!/usr/bin/env python3
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# Copyright the Collabora Online contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

"""Prints Collabora Office Gerrit statistics for the current ISO week."""

import datetime
import json
import sys
import urllib.request
from pathlib import Path

GERRIT_BASE = "https://gerrit.collaboraoffice.com"
PAGE_SIZE = 500

GITHUB_API = "https://api.github.com"
GITHUB_REPO = "CollaboraOnline/online"
GITHUB_ISSUES_URL = f"https://github.com/{GITHUB_REPO}/issues"

JENKINS_BASE = "https://cpci.cbg.collabora.co.uk:8080"
JENKINS_JOB_URL = f"{JENKINS_BASE}/view/Gerrit/job/Gerrit%20for%20online%20main/"
JENKINS_RECENT_BUILD_LIMIT = 50

STATS_FILE = "stats.json"


def fetch_changes(query):
    """Returns all Gerrit changes matching the query, paginating as needed."""
    changes = []
    start = 0
    while True:
        url = f"{GERRIT_BASE}/changes/?q={query}&n={PAGE_SIZE}&start={start}"
        with urllib.request.urlopen(url) as resp:
            body = resp.read().decode("utf-8")
        # Gerrit prefixes JSON responses with )]}' to defeat XSSI.
        if body.startswith(")]}'"):
            body = body.split("\n", 1)[1]
        page = json.loads(body)
        changes.extend(page)
        if not page or not page[-1].get("_more_changes"):
            break
        start += PAGE_SIZE
    return changes


def open_non_wip_count():
    """Number of open Gerrit changes that aren't marked work-in-progress."""
    return len(fetch_changes("status:open+-is:wip"))


def github_issue_count(query):
    """Returns the total_count of GitHub issues matching the search query."""
    url = f"{GITHUB_API}/search/issues?q={query}&per_page=1"
    req = urllib.request.Request(url, headers={"User-Agent": "cool_tc_stats"})
    with urllib.request.urlopen(req) as resp:
        body = resp.read().decode("utf-8")
    return json.loads(body)["total_count"]


def regression_issue_count():
    """Number of open GitHub issues with the 'regression' label."""
    return github_issue_count(f"is:issue+state:open+label:regression+repo:{GITHUB_REPO}")


def open_pr_count():
    """Number of open GitHub pull requests."""
    return github_issue_count(f"is:pr+state:open+repo:{GITHUB_REPO}")


def jenkins_recent_build_success(limit=JENKINS_RECENT_BUILD_LIMIT):
    """Returns (green, total) for the last `limit` completed Jenkins builds.

    A build counts as completed once its result is non-null (i.e. it is no
    longer queued or running). "Green" means result == "SUCCESS".
    """
    url = f"{JENKINS_JOB_URL}api/json?tree=builds%5Bresult%5D"
    req = urllib.request.Request(url, headers={"User-Agent": "cool_tc_stats"})
    with urllib.request.urlopen(req) as resp:
        body = resp.read().decode("utf-8")
    results = [b.get("result") for b in json.loads(body).get("builds", [])]
    completed = [r for r in results if r is not None][:limit]
    green = sum(1 for r in completed if r == "SUCCESS")
    return green, len(completed)


def read_previous_week_stats(current_week_key):
    """Returns the most recent week's stats dict from stats.json that precedes
    current_week_key, or None if there isn't one."""
    path = Path(STATS_FILE)
    if not path.exists():
        return None
    with path.open() as f:
        stats = json.load(f)
    prior_keys = sorted(k for k in stats if k < current_week_key)
    if not prior_keys:
        return None
    return stats[prior_keys[-1]]


def fmt_delta(current, previous):
    """Renders 'current' if there's no prior value, else 'current(+delta)'."""
    if previous is None:
        return str(current)
    return f"{current}({current - previous:+d})"


def write_stats_json(week_key, gerrit_count, pr_count, regression_count):
    """Merges this week's counts into stats.json (preserving older weeks)."""
    path = Path(STATS_FILE)
    if path.exists():
        with path.open() as f:
            stats = json.load(f)
    else:
        stats = {}
    stats[week_key] = {
        "gerritChangesOpenCount": gerrit_count,
        "githubPrOpenCount": pr_count,
        "githubIssueOpenRegressionCount": regression_count,
    }
    with path.open("w") as f:
        json.dump(stats, f, indent=2, sort_keys=True)
        f.write("\n")


def main():
    iso = datetime.date.today().isocalendar()
    week = iso.week
    week_key = f"{iso.year}-W{week:02d}"

    gerrit_count = open_non_wip_count()
    pr_count = open_pr_count()
    regression_count = regression_issue_count()
    jenkins_green, jenkins_total = jenkins_recent_build_success()
    jenkins_success_rate = (
        round(100 * jenkins_green / jenkins_total) if jenkins_total else 0
    )
    jenkins_failed = jenkins_total - jenkins_green

    prev = read_previous_week_stats(week_key) or {}
    gerrit_str = fmt_delta(gerrit_count, prev.get("gerritChangesOpenCount"))
    pr_str = fmt_delta(pr_count, prev.get("githubPrOpenCount"))
    regression_str = fmt_delta(regression_count, prev.get("githubIssueOpenRegressionCount"))

    print("# Patch review")
    print(f"- [All changes]({GERRIT_BASE}/q/status:open+-is:wip)")
    print(f"  - Week {week}: {gerrit_str}")
    print(f"- [PRs to migrate from GitHub](https://github.com/{GITHUB_REPO}/pulls)")
    print(f"  - Week {week}: {pr_str}")
    print("# Bug reporting")
    print(f"- [Regression issues]({GITHUB_ISSUES_URL}?q=is%3Aissue%20state%3Aopen%20label%3Aregression)")
    print(f"  - Week {week}: {regression_str}")
    print("# Jenkins / CI update")
    print(f"- [Gerrit for online main]({JENKINS_JOB_URL})")
    print(f"  - Week {week}: Success rate is {jenkins_success_rate}%, failed builds are {jenkins_failed}/{jenkins_total}")

    write_stats_json(week_key, gerrit_count, pr_count, regression_count)
    return 0


if __name__ == "__main__":
    sys.exit(main())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
