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

"""Prints Collabora Office Gerrit statistics for the current ISO week."""

import collections
import datetime
import json
import re
import statistics
import subprocess
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
JENKINS_PAGE_SIZE = 500

STATS_FILE = "stats.json"

GERRIT_VERDICT = re.compile(r"Build (Successful|Failed|Aborted|Unstable)")


def fetch_changes(query, options=""):
    """Returns all Gerrit changes matching the query, paginating as needed.
    options is appended to the URL as is, e.g. "&o=MESSAGES"."""
    changes = []
    start = 0
    while True:
        url = f"{GERRIT_BASE}/changes/?q={query}&n={PAGE_SIZE}&start={start}{options}"
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
    """Number of open Gerrit changes on the online project's main branch that
    aren't marked work-in-progress."""
    return len(fetch_changes("status:open+-is:wip+project:online+branch:main"))


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


def git_authors(until=None):
    """Returns the set of unique git authors in the current repo's log,
    optionally filtered with --until."""
    cmd = ["git", "log", "--format=%aN"]
    if until:
        cmd.append(f"--until={until}")
    out = subprocess.check_output(cmd, text=True)
    return {line for line in out.splitlines() if line.strip()}


def new_contributors_this_week():
    """Authors who appear in the full git log but not in
    git log --until=1.week.ago, i.e. first commit within the past week."""
    return sorted(git_authors() - git_authors(until="1.week.ago"))


def top_reviewers_this_week(n=10):
    """Top n Reviewed-by trailer authors from commits in the past week,
    as a list of (name, count) tuples sorted by count descending."""
    out = subprocess.check_output(
        ["git", "log", "--no-merges", "--since=1.week.ago",
         "--format=%(trailers:key=Reviewed-by,valueonly,separator=%x0A)"],
        text=True,
    )
    counter = collections.Counter()
    for line in out.splitlines():
        line = line.strip()
        if not line:
            continue
        # "Foo Bar <foo@example.com>" -> "Foo Bar"
        i = line.find("<")
        name = line[:i].strip() if i > 0 else line
        if name:
            counter[name] += 1
    return counter.most_common(n)


def fetch_jenkins_builds_since(since):
    """Returns the Jenkins builds that started at or after `since` (a naive UTC datetime), newest
    first, each with its result, duration and timestamp. Also returns True if the window is
    complete, or False if Jenkins no longer keeps the oldest builds of the window.

    The "builds" field of the job API holds only the newest 100 builds, so this reads
    "allBuilds" one page at a time, until a page reaches a build older than `since`.
    """
    since_ms = since.replace(tzinfo=datetime.timezone.utc).timestamp() * 1000
    builds = []
    start = 0
    while True:
        tree = (f"allBuilds%5Bresult,duration,timestamp%5D"
                f"%7B{start},{start + JENKINS_PAGE_SIZE}%7D")
        url = f"{JENKINS_JOB_URL}api/json?tree={tree}"
        req = urllib.request.Request(url, headers={"User-Agent": "cool_tc_stats"})
        with urllib.request.urlopen(req) as resp:
            body = resp.read().decode("utf-8")
        page = json.loads(body).get("allBuilds", [])
        builds.extend(b for b in page if b["timestamp"] >= since_ms)
        if any(b["timestamp"] < since_ms for b in page):
            return builds, True
        if len(page) < JENKINS_PAGE_SIZE:
            return builds, False
        start += JENKINS_PAGE_SIZE


def jenkins_recent_build_stats(days=7):
    """Returns (green, total, median_duration_ms, median_sample_size, oldest, complete) for the
    completed Jenkins builds that started in the last `days` days.

    A build counts as completed once its result is non-null, so builds still queued or running
    are left out. "Green" means the result is SUCCESS. The median takes only SUCCESS and FAILURE
    builds, because their duration is a real turnaround a contributor would have seen. An
    ABORTED build is usually one cancelled by a newer patchset.

    oldest is the start time of the oldest build counted, as a naive UTC datetime, or None if
    there were no builds. complete is False when Jenkins no longer keeps the start of the window,
    so the figures cover only the time since oldest.
    """
    now = datetime.datetime.now(datetime.timezone.utc).replace(tzinfo=None)
    builds, complete = fetch_jenkins_builds_since(now - datetime.timedelta(days=days))
    completed = [b for b in builds if b.get("result") is not None]
    green = sum(1 for b in completed if b.get("result") == "SUCCESS")
    durations = [
        b["duration"]
        for b in completed
        if b.get("result") in ("SUCCESS", "FAILURE") and b.get("duration")
    ]
    median_ms = int(statistics.median(durations)) if durations else 0
    oldest = None
    if builds:
        oldest = datetime.datetime.fromtimestamp(
            builds[-1]["timestamp"] / 1000, datetime.timezone.utc).replace(tzinfo=None)
    return green, len(completed), median_ms, len(durations), oldest, complete


def parse_gerrit_time(stamp):
    """Parses a Gerrit timestamp such as '2026-09-30 13:48:26.000000000', which is UTC."""
    return datetime.datetime.strptime(stamp[:19], "%Y-%m-%d %H:%M:%S")


def lone_push_to_verdict_stats(days=7):
    """Returns (median_minutes, within_hour_percent, count) for the lone pushes to online main in
    the last `days` days, or (None, None, 0) if there were none.

    The time runs from uploading a patchset to its first Jenkins verdict, read from the change
    messages. A lone push is one made while its author had no other patchset waiting for a
    verdict. A patchset replaced by a newer one before its verdict arrived had its build
    cancelled, so it is left out of the figures, but it still counts as waiting.
    """
    now = datetime.datetime.now(datetime.timezone.utc).replace(tzinfo=None)
    since = now - datetime.timedelta(days=days)
    changes = fetch_changes(
        f"project:online+branch:main+after:{since:%Y-%m-%d}", "&o=MESSAGES")

    # (owner, push, verdict, superseded) for each patchset that got a verdict.
    rows = []
    for change in changes:
        pushes = {}
        verdicts = {}
        for message in change.get("messages", []):
            patchset = message.get("_revision_number")
            when = parse_gerrit_time(message["date"])
            if message["message"].startswith("Uploaded patch set"):
                pushes.setdefault(patchset, when)
            elif (patchset in pushes and patchset not in verdicts
                  and GERRIT_VERDICT.search(message["message"])):
                verdicts[patchset] = when
        owner = change.get("owner", {}).get("_account_id")
        for patchset, verdict in verdicts.items():
            later = [pushes[p] for p in pushes if p > patchset]
            superseded = bool(later) and min(later) < verdict
            rows.append((owner, pushes[patchset], verdict, superseded))

    minutes = []
    for index, (owner, push, verdict, superseded) in enumerate(rows):
        if superseded or push < since:
            continue
        lone = not any(
            other_index != index and other_owner == owner
            and other_push <= push < other_verdict
            for other_index, (other_owner, other_push, other_verdict, _) in enumerate(rows))
        if lone:
            minutes.append((verdict - push).total_seconds() / 60)
    if not minutes:
        return None, None, 0
    within_hour = round(100 * sum(1 for m in minutes if m <= 60) / len(minutes))
    return statistics.median(minutes), within_hour, len(minutes)


def fmt_duration_ms(ms):
    """Renders a millisecond duration as 'X hr Y min', matching how the
    Jenkins UI prints the 'Took ...' line on a build page."""
    minutes = ms // 60_000
    hours, minutes = divmod(minutes, 60)
    if hours:
        return f"{hours} hr {minutes} min"
    return f"{minutes} min"


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
    (jenkins_green, jenkins_total, jenkins_median_ms, jenkins_median_n,
     jenkins_oldest, jenkins_complete) = jenkins_recent_build_stats()
    if jenkins_complete or jenkins_oldest is None:
        jenkins_window = "in the last 7 days"
    else:
        # Jenkins discards the oldest builds of the job, so the window starts later.
        jenkins_window = f"since {jenkins_oldest:%Y-%m-%d %H:%M} UTC, the oldest build Jenkins keeps"
    jenkins_success_rate = (
        round(100 * jenkins_green / jenkins_total) if jenkins_total else 0
    )
    jenkins_failed = jenkins_total - jenkins_green
    lone_median, lone_within_hour, lone_count = lone_push_to_verdict_stats()

    prev = read_previous_week_stats(week_key) or {}
    gerrit_str = fmt_delta(gerrit_count, prev.get("gerritChangesOpenCount"))
    pr_str = fmt_delta(pr_count, prev.get("githubPrOpenCount"))
    regression_str = fmt_delta(regression_count, prev.get("githubIssueOpenRegressionCount"))

    new_contribs = new_contributors_this_week()
    top_reviewers = top_reviewers_this_week()

    print("# Patch review")
    print(f"- [All changes]({GERRIT_BASE}/q/status:open+-is:wip+project:online+branch:main)")
    print(f"  - Week {week}: {gerrit_str}")
    print(f"- [GitHub PRs](https://github.com/{GITHUB_REPO}/pulls)")
    print(f"  - Week {week}: {pr_str}")
    if new_contribs:
        print("- New contributors since last week")
        print(f"  - Week {week}: {', '.join(new_contribs)}")
    if top_reviewers:
        reviewers_str = ", ".join(f"{name} ({count})" for name, count in top_reviewers)
        print("- Top 10 reviewers since last week")
        print(f"  - Week {week}: {reviewers_str}")
    print("# Bug reporting")
    print(f"- [Regression issues]({GITHUB_ISSUES_URL}?q=is%3Aissue%20state%3Aopen%20label%3Aregression)")
    print(f"  - Week {week}: {regression_str}")
    print("# Jenkins / CI update")
    print(f"- [Gerrit for online main]({JENKINS_JOB_URL})")
    print(f"  - Week {week}: Success rate is {jenkins_success_rate}%, failed builds are {jenkins_failed}/{jenkins_total} ({jenkins_window})")
    print(f"  - Week {week}: Turnaround time is {fmt_duration_ms(jenkins_median_ms)} (median of {jenkins_median_n} SUCCESS/FAILURE builds {jenkins_window})")
    if lone_count:
        print(f"  - Week {week}: {lone_within_hour}% of lone pushes got a CI verdict within an hour"
              f" ({lone_count} lone pushes in the last 7 days, median"
              f" {fmt_duration_ms(int(lone_median * 60_000))})")

    write_stats_json(week_key, gerrit_count, pr_count, regression_count)
    return 0


if __name__ == "__main__":
    sys.exit(main())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
